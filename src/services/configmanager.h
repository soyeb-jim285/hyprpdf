#pragma once
#include <QObject>
#include <QString>
#include <QTimer>

class ConfigManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString theme               READ theme               WRITE setTheme               NOTIFY configChanged)
    Q_PROPERTY(QString iconTheme           READ iconTheme           WRITE setIconTheme           NOTIFY configChanged)
    Q_PROPERTY(double  defaultZoom         READ defaultZoom         WRITE setDefaultZoom         NOTIFY configChanged)
    Q_PROPERTY(bool    continuousScroll    READ continuousScroll    WRITE setContinuousScroll    NOTIFY configChanged)
    Q_PROPERTY(int     radiusSm            READ radiusSm            WRITE setRadiusSm            NOTIFY configChanged)
    Q_PROPERTY(int     radiusMd            READ radiusMd            WRITE setRadiusMd            NOTIFY configChanged)
    Q_PROPERTY(int     radiusLg            READ radiusLg            WRITE setRadiusLg            NOTIFY configChanged)
    Q_PROPERTY(bool    animationsEnabled   READ animationsEnabled   WRITE setAnimationsEnabled   NOTIFY configChanged)
    Q_PROPERTY(int     animDurationFast    READ animDurationFast    WRITE setAnimDurationFast    NOTIFY configChanged)
    Q_PROPERTY(int     animDurationNormal  READ animDurationNormal  WRITE setAnimDurationNormal  NOTIFY configChanged)
    Q_PROPERTY(int     animDurationSlow    READ animDurationSlow    WRITE setAnimDurationSlow    NOTIFY configChanged)
    Q_PROPERTY(bool    transparencyEnabled READ transparencyEnabled WRITE setTransparencyEnabled NOTIFY configChanged)
    Q_PROPERTY(double  transparencyLevel   READ transparencyLevel   WRITE setTransparencyLevel   NOTIFY configChanged)

public:
    explicit ConfigManager(const QString &path = {}, QObject *parent = nullptr);

    QString theme()            const { return m_theme; }
    QString iconTheme()        const { return m_iconTheme; }
    double  defaultZoom()      const { return m_defaultZoom; }
    bool    continuousScroll() const { return m_continuousScroll; }
    int     radiusSm()         const { return m_radiusSm; }
    int     radiusMd()         const { return m_radiusMd; }
    int     radiusLg()         const { return m_radiusLg; }
    bool    animationsEnabled()   const { return m_animationsEnabled; }
    int     animDurationFast()    const { return m_animDurationFast; }
    int     animDurationNormal()  const { return m_animDurationNormal; }
    int     animDurationSlow()    const { return m_animDurationSlow; }
    bool    transparencyEnabled() const { return m_transparencyEnabled; }
    double  transparencyLevel()   const { return m_transparencyLevel; }

    void setTheme(const QString &v);
    void setIconTheme(const QString &v);
    void setDefaultZoom(double v);
    void setContinuousScroll(bool v);
    void setRadiusSm(int v);
    void setRadiusMd(int v);
    void setRadiusLg(int v);
    void setAnimationsEnabled(bool v);
    void setAnimDurationFast(int v);
    void setAnimDurationNormal(int v);
    void setAnimDurationSlow(int v);
    void setTransparencyEnabled(bool v);
    void setTransparencyLevel(double v);

    Q_INVOKABLE void save();
    Q_INVOKABLE void reload();

signals:
    void configChanged();

private:
    void load();
    void scheduleSave();
    void rotateBroken();

    QString m_path;
    QTimer  m_saveTimer;

    QString m_theme = "default";
    QString m_iconTheme = "default";
    double  m_defaultZoom = 1.0;
    bool    m_continuousScroll = true;
    int     m_radiusSm = 4;
    int     m_radiusMd = 8;
    int     m_radiusLg = 12;
    bool    m_animationsEnabled = true;
    int     m_animDurationFast = 100;
    int     m_animDurationNormal = 200;
    int     m_animDurationSlow = 350;
    bool    m_transparencyEnabled = false;
    double  m_transparencyLevel = 0.85;
};
