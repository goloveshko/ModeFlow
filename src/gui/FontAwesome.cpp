#include "FontAwesome.h"

#include <QApplication>
#include <QFontDatabase>
#include <QPainter>

#include "Logging.h"
#include "StyleBridge.h"

using namespace Qt::StringLiterals;

namespace ModeFlow::Gui {

namespace {
constexpr int DefaultIconSizePx = 16;
constexpr int DevicePixelRatioPrecision = 100;

QHash<QString, QIcon>& iconCache() {
    static QHash<QString, QIcon> cache;
    return cache;
}

qreal currentDevicePixelRatio() {
    return qApp ? qApp->devicePixelRatio() : 1.0;
}

int devicePixelRatioBucket(qreal devicePixelRatio) {
    return qRound(devicePixelRatio * DevicePixelRatioPrecision);
}

QString iconCacheKey(const QString& symbol, int size, quint64 themeRevision, int dprBucket) {
    return symbol % u"_"_sv % QString::number(size) % u"_"_sv % QString::number(themeRevision) % u"_"_sv %
           QString::number(dprBucket);
}
} // namespace

QString FontAwesome::s_fontFamily = u""_s;
quint64 FontAwesome::s_themeRevision = 0;

void FontAwesome::ensureFontLoaded() {
    if (!s_fontFamily.isEmpty())
        return;

    int fontId = QFontDatabase::addApplicationFont(u":/fonts/Font Awesome 7 Free-Solid-900.otf"_s);
    if (fontId != -1) {
        s_fontFamily = QFontDatabase::applicationFontFamilies(fontId).at(0);
    } else {
        qCWarning(lcGui) << "Could not load font file!";
        s_fontFamily = u"sans-serif"_s; // Fallback
    }
}

QString FontAwesome::fontFamily() {
    ensureFontLoaded();
    return s_fontFamily;
}

QIcon FontAwesome::icon(const QString& symbol, int size) {
    ensureFontLoaded();

    const int targetSize = size > 0 ? size : DefaultIconSizePx;
    const qreal devicePixelRatio = currentDevicePixelRatio();
    const int dprBucket = devicePixelRatioBucket(devicePixelRatio);
    auto& cache = iconCache();
    const QString cacheKey = iconCacheKey(symbol, targetSize, s_themeRevision, dprBucket);
    auto it = cache.constFind(cacheKey);
    if (it != cache.constEnd())
        return it.value();

    auto& bridge = StyleBridge::instance();
    QColor normalColor = bridge.iconNormal();
    QColor activeColor = bridge.iconActive();
    QColor disabledColor = bridge.iconDisabled();

    auto generatePixmap = [&](const QColor& color, int targetSize) {
        const int physicalSize = qMax(1, qRound(targetSize * devicePixelRatio));
        QPixmap pixmap(physicalSize, physicalSize);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::TextAntialiasing);
        painter.setPen(color);

        QFont font(s_fontFamily);
        font.setPixelSize(physicalSize);
        painter.setFont(font);

        painter.drawText(pixmap.rect(), Qt::AlignCenter, symbol);
        painter.end();
        pixmap.setDevicePixelRatio(devicePixelRatio);
        return pixmap;
    };

    QIcon icon;
    icon.addPixmap(generatePixmap(normalColor, targetSize), QIcon::Normal, QIcon::Off);
    icon.addPixmap(generatePixmap(activeColor, targetSize), QIcon::Active, QIcon::Off);
    icon.addPixmap(generatePixmap(disabledColor, targetSize), QIcon::Disabled, QIcon::Off);

    cache.insert(cacheKey, icon);
    return icon;
}

void FontAwesome::invalidateCache() {
    ++s_themeRevision;
    iconCache().clear();
}

QList<FontAwesome::IconDefinition> FontAwesome::profileIconDefinitions() {
    return {
        {u"desktop"_s, QObject::tr("Desktop"), Desktop},
        {u"monitor"_s, QObject::tr("Monitor"), Monitor},
        {u"tv"_s, QObject::tr("TV"), Tv},
        {u"laptop"_s, QObject::tr("Laptop"), Laptop},
        {u"audio"_s, QObject::tr("Audio"), Audio},
        {u"music"_s, QObject::tr("Music"), Music},
        {u"video"_s, QObject::tr("Video"), Video},
        {u"gamepad"_s, QObject::tr("Gaming"), Gamepad},
        {u"briefcase"_s, QObject::tr("Work"), Briefcase},
        {u"house"_s, QObject::tr("Home"), House},
    };
}

QString FontAwesome::defaultProfileIconSymbol() {
    return Desktop;
}

} // namespace ModeFlow::Gui
