#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QColor>

/**
 * @brief Theme Manager for the GUI application
 * 
 * Manages application theming with support for dark/light modes,
 * custom colors, and theme persistence.
 */
class ThemeManager : public QObject {
    Q_OBJECT
    
    Q_PROPERTY(bool darkTheme READ darkTheme WRITE setDarkTheme NOTIFY darkThemeChanged)
    Q_PROPERTY(QString primaryColor READ primaryColor WRITE setPrimaryColor NOTIFY primaryColorChanged)
    Q_PROPERTY(QString accentColor READ accentColor WRITE setAccentColor NOTIFY accentColorChanged)
    Q_PROPERTY(QString successColor READ successColor WRITE setSuccessColor NOTIFY successColorChanged)
    Q_PROPERTY(QString warningColor READ warningColor WRITE setWarningColor NOTIFY warningColorChanged)
    Q_PROPERTY(QString backgroundColor READ backgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(QString surfaceColor READ surfaceColor NOTIFY surfaceColorChanged)
    Q_PROPERTY(QString textColor READ textColor NOTIFY textColorChanged)
    Q_PROPERTY(QString secondaryTextColor READ secondaryTextColor NOTIFY secondaryTextColorChanged)
    
    Q_PROPERTY(QStringList customColorNames READ customColorNames NOTIFY customColorsChanged)

public:
    explicit ThemeManager(QObject* parent = nullptr);
    ~ThemeManager();

    // Properties
    bool darkTheme() const { return m_darkTheme; }
    void setDarkTheme(bool dark);

    QString primaryColor() const { return m_primaryColor; }
    void setPrimaryColor(const QString& color) {
        if (m_primaryColor != color) {
            m_primaryColor = color;
            emit primaryColorChanged();
            saveTheme();
        }
    }

    QString accentColor() const { return m_accentColor; }
    void setAccentColor(const QString& color) {
        if (m_accentColor != color) {
            m_accentColor = color;
            emit accentColorChanged();
            saveTheme();
        }
    }

    QString successColor() const { return m_successColor; }
    void setSuccessColor(const QString& color) {
        if (m_successColor != color) {
            m_successColor = color;
            emit successColorChanged();
            saveTheme();
        }
    }

    QString warningColor() const { return m_warningColor; }
    void setWarningColor(const QString& color) {
        if (m_warningColor != color) {
            m_warningColor = color;
            emit warningColorChanged();
            saveTheme();
        }
    }

    QString backgroundColor() const { return m_backgroundColor; }
    QString surfaceColor() const { return m_surfaceColor; }
    QString textColor() const { return m_textColor; }
    QString secondaryTextColor() const { return m_secondaryTextColor; }
    
    QStringList customColorNames() const { return m_customColors.keys(); }

    // Public methods
    Q_INVOKABLE void initialize();
    Q_INVOKABLE void toggleTheme();
    Q_INVOKABLE void applyTheme();
    Q_INVOKABLE void applyQMLTheme();
    
    Q_INVOKABLE bool loadTheme();
    Q_INVOKABLE bool saveTheme();
    Q_INVOKABLE void resetToDefaults();
    
    Q_INVOKABLE QString getCustomColor(const QString& name, const QString& defaultValue = QString()) const;
    Q_INVOKABLE void setCustomColor(const QString& name, const QString& color);
    
    Q_INVOKABLE QColor getColor(const QString& name) const;
    Q_INVOKABLE QString getColorString(const QString& name) const;

signals:
    void darkThemeChanged();
    void primaryColorChanged();
    void accentColorChanged();
    void successColorChanged();
    void warningColorChanged();
    void backgroundColorChanged();
    void surfaceColorChanged();
    void textColorChanged();
    void secondaryTextColorChanged();
    void customColorsChanged();
    void customColorChanged(const QString& name, const QString& color);
    void themeChanged(bool dark);
    void themeApplied();
    void error(const QString& message);

private:
    void updateColors();
    QString getDefaultBackgroundColor() const;
    QString getDefaultSurfaceColor() const;
    QString getDefaultTextColor() const;
    QString getDefaultSecondaryTextColor() const;

    // Theme settings
    bool m_darkTheme;
    QString m_primaryColor;
    QString m_accentColor;
    QString m_successColor;
    QString m_warningColor;
    
    // Computed colors
    QString m_backgroundColor;
    QString m_surfaceColor;
    QString m_textColor;
    QString m_secondaryTextColor;
    
    // Custom colors
    QMap<QString, QString> m_customColors;
    
    // File management
    QString m_themeFile;
};

#endif // THEME_MANAGER_H
