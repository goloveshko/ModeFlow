#include "TaskScheduler.h"

#include <QCoreApplication>
#include <QDir>

#include "Constants.h"
#include "Logging.h"

#ifdef Q_OS_WIN
#include <comdef.h>
#include <taskschd.h>
#include <winsvc.h>
#include <wrl/client.h>
#endif

using Microsoft::WRL::ComPtr;

namespace ModeFlow::Utils {

using namespace Qt::StringLiterals;

// Helper to convert HRESULT to string for logging
#ifdef Q_OS_WIN
static QString hresultToString(HRESULT hr) {
    _com_error err(hr);
    return QString::fromWCharArray(err.ErrorMessage());
}
#endif

namespace {
void fillDefaults(QString& taskName, QString& exePath) {
    if (taskName.isEmpty()) {
        taskName = QCoreApplication::applicationName();
        if (taskName.isEmpty()) {
            taskName = QFileInfo(QCoreApplication::applicationFilePath()).baseName();
        }
    }
    if (exePath.isEmpty()) {
        exePath = QCoreApplication::applicationFilePath();
    }
}
} // namespace

bool TaskScheduler::createTaskAtLogon(const QString& arguments, int delaySeconds, bool runHighest, QString taskName,
                                      QString exePath) {
    if (!isAdmin()) {
        qCWarning(lcUtil) << "Administrator rights required";
        return false;
    }

    SC_HANDLE hSCM = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!hSCM) {
        qCWarning(lcUtil) << "Cannot access Task Scheduler service";
        return false;
    }
    CloseServiceHandle(hSCM);

    fillDefaults(taskName, exePath);

    ComPtr<ITaskService> pService;
    ComPtr<ITaskFolder> pRootFolder;
    ComPtr<ITaskDefinition> pTask;
    ComPtr<ITaskSettings> pSettings;
    ComPtr<ITriggerCollection> pTriggerCollection;
    ComPtr<ITrigger> pTrigger;
    ComPtr<ILogonTrigger> pLogonTrigger;
    ComPtr<IActionCollection> pActionCollection;
    ComPtr<IAction> pAction;
    ComPtr<IExecAction> pExecAction;
    ComPtr<IPrincipal> pPrincipal;
    ComPtr<IRegisteredTask> pRegisteredTask;

    HRESULT hr = CoCreateInstance(CLSID_TaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pService));
    if (FAILED(hr)) {
        qCWarning(lcUtil) << "CoCreateInstance failed:" << hresultToString(hr);
        return false;
    }

    hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
    if (FAILED(hr)) {
        qCWarning(lcUtil) << "Connect failed:" << hresultToString(hr);
        return false;
    }

    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr)) {
        qCWarning(lcUtil) << "GetFolder failed:" << hresultToString(hr);
        return false;
    }

    // Remove existing (Ignore failure if it doesn't exist)
    hr = pRootFolder->DeleteTask(_bstr_t(taskName.toStdWString().c_str()), 0);

    if (FAILED(hr) && hr != HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
        // ERROR_ACCESS_DENIED (0x80070005)
        qCWarning(lcUtil) << "Critical error deleting task:" << hresultToString(hr);
        return false;
    }

    hr = pService->NewTask(0, &pTask);
    if (FAILED(hr)) {
        qCWarning(lcUtil) << "NewTask failed:" << hresultToString(hr);
        return false;
    }

    if (SUCCEEDED(pTask->get_Principal(&pPrincipal))) {
        if (runHighest) {
            pPrincipal->put_RunLevel(TASK_RUNLEVEL_HIGHEST);
        } else {
            pPrincipal->put_RunLevel(TASK_RUNLEVEL_LUA);
        }
    }

    if (SUCCEEDED(pTask->get_Settings(&pSettings))) {
        pSettings->put_StartWhenAvailable(VARIANT_TRUE);
        pSettings->put_DisallowStartIfOnBatteries(VARIANT_FALSE);
        pSettings->put_StopIfGoingOnBatteries(VARIANT_FALSE);
        pSettings->put_ExecutionTimeLimit(_bstr_t(L"PT0S"));
        pSettings->put_Priority(5);
    }

    if (SUCCEEDED(pTask->get_Triggers(&pTriggerCollection))) {
        if (SUCCEEDED(pTriggerCollection->Create(TASK_TRIGGER_LOGON, &pTrigger))) {
            if (SUCCEEDED(pTrigger.As(&pLogonTrigger))) {
                pLogonTrigger->put_Id(_bstr_t(L"LogonTrigger"));

                // If a delay is specified, format it as an ISO 8601 duration (PT#S)
                if (delaySeconds > 0) {
                    // Safety clamp: 1 to 30 seconds
                    int clampedDelay = std::clamp(delaySeconds, 1, 30);
                    QString delayStr = u"PT%1S"_s.arg(clampedDelay);
                    pLogonTrigger->put_Delay(_bstr_t(delayStr.toStdWString().c_str()));

                    qCDebug(lcUtil) << "Setting logon delay to" << clampedDelay << "seconds.";
                } else {
                    pLogonTrigger->put_Delay(nullptr); // No delay
                }
            }
        }
    }

    if (SUCCEEDED(pTask->get_Actions(&pActionCollection))) {
        if (SUCCEEDED(pActionCollection->Create(TASK_ACTION_EXEC, &pAction))) {
            if (SUCCEEDED(pAction.As(&pExecAction))) {
                pExecAction->put_Path(_bstr_t(QDir::toNativeSeparators(exePath).toStdWString().c_str()));
                pExecAction->put_Arguments(_bstr_t(arguments.toStdWString().c_str()));

                QString workingDir = QFileInfo(exePath).absolutePath();
                pExecAction->put_WorkingDirectory(_bstr_t(QDir::toNativeSeparators(workingDir).toStdWString().c_str()));
            }
        }
    }

    hr = pRootFolder->RegisterTaskDefinition(_bstr_t(taskName.toStdWString().c_str()), pTask.Get(),
                                             TASK_CREATE_OR_UPDATE, _variant_t(), _variant_t(),
                                             TASK_LOGON_INTERACTIVE_TOKEN, _variant_t(L""), &pRegisteredTask);

    if (FAILED(hr)) {
        qCWarning(lcUtil) << "RegisterTaskDefinition failed:" << hresultToString(hr);
        return false;
    }

    qCDebug(lcUtil) << "Task created successfully:" << taskName;
    return true;
}

bool TaskScheduler::removeTask(QString taskName) {
    QString exePath;
    fillDefaults(taskName, exePath);

    ComPtr<ITaskService> pService;
    ComPtr<ITaskFolder> pRootFolder;

    HRESULT hr = CoCreateInstance(CLSID_TaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pService));
    if (FAILED(hr)) {
        qCWarning(lcUtil) << "CoCreateInstance failed:" << hresultToString(hr);
        return false;
    }

    hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
    if (FAILED(hr)) {
        qCWarning(lcUtil) << "Connect failed:" << hresultToString(hr);
        return false;
    }

    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr)) {
        qCWarning(lcUtil) << "GetFolder failed:" << hresultToString(hr);
        return false;
    }

    hr = pRootFolder->DeleteTask(_bstr_t(taskName.toStdWString().c_str()), 0);
    if (FAILED(hr)) {
        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)) {
            qCDebug(lcUtil) << "Task already absent:" << taskName;
            return true;
        }
        qCWarning(lcUtil) << "DeleteTask failed:" << hresultToString(hr);
        return false;
    }

    qCDebug(lcUtil) << "Task removed successfully:" << taskName;
    return true;
}

bool TaskScheduler::isTaskRegistered(QString taskName, QString exePath) {
    fillDefaults(taskName, exePath);

    ComPtr<ITaskService> pService;
    ComPtr<ITaskFolder> pRootFolder;
    ComPtr<IRegisteredTask> pRegisteredTask;

    HRESULT hr = CoCreateInstance(CLSID_TaskScheduler, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pService));
    if (FAILED(hr)) {
        qCWarning(lcUtil) << "CoCreateInstance failed:" << hresultToString(hr);
        return false;
    }

    hr = pService->Connect(_variant_t(), _variant_t(), _variant_t(), _variant_t());
    if (FAILED(hr)) {
        qCWarning(lcUtil) << "Connect failed:" << hresultToString(hr);
        return false;
    }

    hr = pService->GetFolder(_bstr_t(L"\\"), &pRootFolder);
    if (FAILED(hr)) {
        qCWarning(lcUtil) << "GetFolder failed:" << hresultToString(hr);
        return false;
    }

    hr = pRootFolder->GetTask(_bstr_t(taskName.toStdWString().c_str()), &pRegisteredTask);
    if (FAILED(hr)) {
        // Task not found is not an error in this context
        return false;
    }

    ComPtr<ITaskDefinition> pDefinition;
    hr = pRegisteredTask->get_Definition(&pDefinition);
    if (FAILED(hr)) {
        qCWarning(lcUtil) << "Get definition failed:" << hresultToString(hr);
        return false;
    }

    ComPtr<IActionCollection> pActions;
    hr = pDefinition->get_Actions(&pActions);
    if (FAILED(hr)) {
        qCWarning(lcUtil) << "Get actions failed:" << hresultToString(hr);
        return false;
    }

    // Retrieve the first action (Note: Task Scheduler uses 1-based indexing)
    ComPtr<IAction> pAction;
    hr = pActions->get_Item(1, &pAction);
    if (FAILED(hr)) {
        qCWarning(lcUtil) << "Get item failed:" << hresultToString(hr);
        return false;
    }

    ComPtr<IExecAction> pExecAction;
    hr = pAction.As(&pExecAction);
    if (FAILED(hr)) {
        qCWarning(lcUtil) << "As<IExecAction> failed:" << hresultToString(hr);
        return false;
    }

    BSTR rawPath = nullptr;
    if (SUCCEEDED(pExecAction->get_Path(&rawPath))) {
        _bstr_t pathWrapper(rawPath, false);
        QString registeredPath = QString::fromWCharArray(static_cast<const wchar_t*>(pathWrapper));

        return QString::compare(QDir::toNativeSeparators(exePath), QDir::toNativeSeparators(registeredPath),
                                Qt::CaseInsensitive) == 0;
    } else {
        qCWarning(lcUtil) << "Get path failed:" << hresultToString(hr);
    }

    return false;
}

bool TaskScheduler::isAdmin() {
    BOOL fRet = FALSE;
    HANDLE hToken = nullptr;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION elevation;
        DWORD cbSize = sizeof(TOKEN_ELEVATION);
        if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &cbSize)) {
            fRet = elevation.TokenIsElevated;
        }
    }
    if (hToken)
        CloseHandle(hToken);
    return fRet;
}

bool TaskScheduler::runAsAdmin(const QString& argument) {
    QString path = QCoreApplication::applicationFilePath();
    std::wstring wPath = QDir::toNativeSeparators(path).toStdWString();
    std::wstring wArgs = argument.toStdWString();

    SHELLEXECUTEINFOW sei = {sizeof(sei)};
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.lpVerb = L"runas";
    sei.lpFile = wPath.c_str();
    sei.lpParameters = wArgs.c_str();
    sei.nShow = SW_NORMAL;

    if (!ShellExecuteExW(&sei)) {
        // If the user clicked "No", GetLastError will return ERROR_CANCELLED (1223)
        return false;
    }

    if (!sei.hProcess) {
        return false;
    }

    const DWORD waitResult = WaitForSingleObject(sei.hProcess, ElevatedHelperTimeoutMs);
    if (waitResult != WAIT_OBJECT_0) {
        if (waitResult == WAIT_TIMEOUT) {
            qCWarning(lcUtil) << "Elevated helper timed out after" << ElevatedHelperTimeoutMs << "ms";
        } else {
            qCWarning(lcUtil) << "Failed while waiting for elevated helper. waitResult =" << waitResult;
        }
        CloseHandle(sei.hProcess);
        return false;
    }

    DWORD exitCode = 1;
    if (!GetExitCodeProcess(sei.hProcess, &exitCode)) {
        qCWarning(lcUtil) << "Failed to read elevated helper exit code. Error =" << GetLastError();
        CloseHandle(sei.hProcess);
        return false;
    }

    CloseHandle(sei.hProcess);

    if (exitCode != 0) {
        qCWarning(lcUtil) << "Elevated helper exited with code" << exitCode;
        return false;
    }

    return true;
}

} // namespace ModeFlow::Utils
