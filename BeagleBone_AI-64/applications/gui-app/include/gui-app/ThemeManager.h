#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QColor>
#include <QTimer>
#include <QJsonObject>

/**
 * @brief Theme Manager for the GUI application
 * 
 * Manages application theming with support for:
 * - Dark/Light mode switching
 * - Custom color palettes
 * - Theme persistence to JSON file
 * - Real-time theme updates
 * - QML integration via properties
 * 
 * Usage:
 * @code
 * ThemeManager* theme = new ThemeManager(this);
 * theme->initialize();
 * theme->setDarkTheme(true);
 * QString color = theme->getColorString("primary");
 * @endcode
 */
class ThemeManager : public QObject {
    Q_OBJECT
    
    // Core theme properties
    Q_PROPERTY(bool darkTheme READ darkTheme WRITE setDarkTheme NOTIFY darkThemeChanged)
    Q_PROPERTY(bool initialized READ isInitialized NOTIFY initializedChanged)
    
    // Primary colors
    Q_PROPERTY(QString primaryColor READ primaryColor WRITE setPrimaryColor NOTIFY primaryColorChanged)
    Q_PROPERTY(QString primaryLightColor READ primaryLightColor NOTIFY primaryColorChanged)
    Q_PROPERTY(QString primaryDarkColor READ primaryDarkColor NOTIFY primaryColorChanged)
    
    // Accent colors
    Q_PROPERTY(QString accentColor READ accentColor WRITE setAccentColor NOTIFY accentColorChanged)
    Q_PROPERTY(QString accentLightColor READ accentLightColor NOTIFY accentColorChanged)
    Q_PROPERTY(QString accentDarkColor READ accentDarkColor NOTIFY accentColorChanged)
    
    // Status colors
    Q_PROPERTY(QString successColor READ successColor WRITE setSuccessColor NOTIFY successColorChanged)
    Q_PROPERTY(QString warningColor READ warningColor WRITE setWarningColor NOTIFY warningColorChanged)
    Q_PROPERTY(QString errorColor READ errorColor WRITE setErrorColor NOTIFY errorColorChanged)
    Q_PROPERTY(QString infoColor READ infoColor WRITE setInfoColor NOTIFY infoColorChanged)
    
    // Background colors
    Q_PROPERTY(QString backgroundColor READ backgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(QString backgroundLightColor READ backgroundLightColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(QString backgroundDarkColor READ backgroundDarkColor NOTIFY backgroundColorChanged)
    
    // Surface colors
    Q_PROPERTY(QString surfaceColor READ surfaceColor NOTIFY surfaceColorChanged)
    Q_PROPERTY(QString surfaceLightColor READ surfaceLightColor NOTIFY surfaceColorChanged)
    Q_PROPERTY(QString surfaceDarkColor READ surfaceDarkColor NOTIFY surfaceColorChanged)
    
    // Text colors
    Q_PROPERTY(QString textColor READ textColor NOTIFY textColorChanged)
    Q_PROPERTY(QString textSecondaryColor READ textSecondaryColor NOTIFY textColorChanged)
    Q_PROPERTY(QString textHintColor READ textHintColor NOTIFY textColorChanged)
    Q_PROPERTY(QString textDisabledColor READ textDisabledColor NOTIFY textColorChanged)
    
    // Border colors
    Q_PROPERTY(QString borderColor READ borderColor NOTIFY borderColorChanged)
    Q_PROPERTY(QString borderLightColor READ borderLightColor NOTIFY borderColorChanged)
    Q_PROPERTY(QString borderDarkColor READ borderDarkColor NOTIFY borderColorChanged)
    
    // Shadow colors
    Q_PROPERTY(QString shadowColor READ shadowColor NOTIFY shadowColorChanged)
    Q_PROPERTY(QString shadowLightColor READ shadowLightColor NOTIFY shadowColorChanged)
    Q_PROPERTY(QString shadowDarkColor READ shadowDarkColor NOTIFY shadowColorChanged)
    
    // Component colors
    Q_PROPERTY(QString cardColor READ cardColor NOTIFY cardColorChanged)
    Q_PROPERTY(QString dialogColor READ dialogColor NOTIFY dialogColorChanged)
    Q_PROPERTY(QString menuColor READ menuColor NOTIFY menuColorChanged)
    Q_PROPERTY(QString tooltipColor READ tooltipColor NOTIFY tooltipColorChanged)
    
    // Custom colors
    Q_PROPERTY(QStringList customColorNames READ customColorNames NOTIFY customColorsChanged)

public:
    /**
     * @brief Constructor
     * @param parent Parent QObject
     */
    explicit ThemeManager(QObject* parent = nullptr);
    
    /**
     * @brief Destructor
     */
    ~ThemeManager();

    // ========================================================================
    // Properties Getters
    // ========================================================================
    
    bool darkTheme() const { return m_darkTheme; }
    bool isInitialized() const { return m_initialized; }
    
    // Primary colors
    QString primaryColor() const { return m_primaryColor; }
    QString primaryLightColor() const { return m_primaryLightColor; }
    QString primaryDarkColor() const { return m_primaryDarkColor; }
    
    // Accent colors
    QString accentColor() const { return m_accentColor; }
    QString accentLightColor() const { return m_accentLightColor; }
    QString accentDarkColor() const { return m_accentDarkColor; }
    
    // Status colors
    QString successColor() const { return m_successColor; }
    QString warningColor() const { return m_warningColor; }
    QString errorColor() const { return m_errorColor; }
    QString infoColor() const { return m_infoColor; }
    
    // Background colors
    QString backgroundColor() const { return m_backgroundColor; }
    QString backgroundLightColor() const { return m_backgroundLightColor; }
    QString backgroundDarkColor() const { return m_backgroundDarkColor; }
    
    // Surface colors
    QString surfaceColor() const { return m_surfaceColor; }
    QString surfaceLightColor() const { return m_surfaceLightColor; }
    QString surfaceDarkColor() const { return m_surfaceDarkColor; }
    
    // Text colors
    QString textColor() const { return m_textColor; }
    QString textSecondaryColor() const { return m_textSecondaryColor; }
    QString textHintColor() const { return m_textHintColor; }
    QString textDisabledColor() const { return m_textDisabledColor; }
    
    // Border colors
    QString borderColor() const { return m_borderColor; }
    QString borderLightColor() const { return m_borderLightColor; }
    QString borderDarkColor() const { return m_borderDarkColor; }
    
    // Shadow colors
    QString shadowColor() const { return m_shadowColor; }
    QString shadowLightColor() const { return m_shadowLightColor; }
    QString shadowDarkColor() const { return m_shadowDarkColor; }
    
    // Component colors
    QString cardColor() const { return m_cardColor; }
    QString dialogColor() const { return m_dialogColor; }
    QString menuColor() const { return m_menuColor; }
    QString tooltipColor() const { return m_tooltipColor; }
    
    QStringList customColorNames() const { return m_customColors.keys(); }

    // ========================================================================
    // Properties Setters
    // ========================================================================
    
    void setDarkTheme(bool dark);
    
    void setPrimaryColor(const QString& color);
    void setAccentColor(const QString& color);
    void setSuccessColor(const QString& color);
    void setWarningColor(const QString& color);
    void setErrorColor(const QString& color);
    void setInfoColor(const QString& color);

    // ========================================================================
    // Public Methods
    // ========================================================================
    
    /**
     * @brief Initialize the theme manager
     * Loads saved theme or applies defaults
     */
    Q_INVOKABLE void initialize();
    
    /**
     * @brief Toggle between dark and light themes
     */
    Q_INVOKABLE void toggleTheme();
    
    /**
     * @brief Apply current theme colors
     * Updates all color properties and emits change signals
     */
    Q_INVOKABLE void applyTheme();
    
    /**
     * @brief Apply theme to QML engine
     * Sets QML style and global properties
     */
    Q_INVOKABLE void applyQMLTheme();
    
    /**
     * @brief Load theme from file
     * @return true if loaded successfully
     */
    Q_INVOKABLE bool loadTheme();
    
    /**
     * @brief Save theme to file
     * @return true if saved successfully
     */
    Q_INVOKABLE bool saveTheme();
    
    /**
     * @brief Reset theme to defaults
     */
    Q_INVOKABLE void resetToDefaults();
    
    /**
     * @brief Get a custom color by name
     * @param name Color name
     * @param defaultValue Default value if color not found
     * @return Color string or default
     */
    Q_INVOKABLE QString getCustomColor(const QString& name, const QString& defaultValue = QString()) const;
    
    /**
     * @brief Set a custom color
     * @param name Color name
     * @param color Color value
     */
    Q_INVOKABLE void setCustomColor(const QString& name, const QString& color);
    
    /**
     * @brief Get color as QColor
     * @param name Color name
     * @return QColor object
     */
    Q_INVOKABLE QColor getColor(const QString& name) const;
    
    /**
     * @brief Get color as string
     * @param name Color name
     * @return Color string
     */
    Q_INVOKABLE QString getColorString(const QString& name) const;
    
    /**
     * @brief Get theme JSON representation
     * @return JSON object with theme data
     */
    Q_INVOKABLE QJsonObject getThemeJson() const;
    
    /**
     * @brief Load theme from JSON
     * @param json JSON object with theme data
     * @return true if loaded successfully
     */
    Q_INVOKABLE bool loadThemeJson(const QJsonObject& json);
    
    /**
     * @brief Get theme file path
     */
    Q_INVOKABLE QString getThemeFile() const { return m_themeFile; }
    
    /**
     * @brief Set theme file path
     * @param path Path to theme file
     */
    Q_INVOKABLE void setThemeFile(const QString& path);

signals:
    // Core signals
    void darkThemeChanged();
    void initializedChanged();
    
    // Color changed signals
    void primaryColorChanged();
    void accentColorChanged();
    void successColorChanged();
    void warningColorChanged();
    void errorColorChanged();
    void infoColorChanged();
    
    void backgroundColorChanged();
    void surfaceColorChanged();
    void textColorChanged();
    void borderColorChanged();
    void shadowColorChanged();
    
    void cardColorChanged();
    void dialogColorChanged();
    void menuColorChanged();
    void tooltipColorChanged();
    
    // Custom colors
    void customColorsChanged();
    void customColorChanged(const QString& name, const QString& color);
    
    // Combined signals
    void themeChanged(bool dark);
    void themeApplied();
    void themeLoaded();
    void themeSaved();
    void error(const QString& message);

private:
    // ========================================================================
    // Private Methods
    // ========================================================================
    
    void initDefaults();
    void updateColors();
    void updateDerivedColors();
    void emitAllColorSignals();
    
    QString getDefaultPrimaryColor() const;
    QString getDefaultAccentColor() const;
    QString getDefaultSuccessColor() const;
    QString getDefaultWarningColor() const;
    QString getDefaultErrorColor() const;
    QString getDefaultInfoColor() const;
    
    QString adjustBrightness(const QString& color, int percent) const;
    QString lightenColor(const QString& color, int percent) const;
    QString darkenColor(const QString& color, int percent) const;
    QString getBackgroundColor() const;
    QString getSurfaceColor() const;
    QString getTextColor() const;
    QString getTextSecondaryColor() const;
    QString getTextHintColor() const;
    QString getTextDisabledColor() const;
    QString getBorderColor() const;
    QString getShadowColor() const;
    QString getCardColor() const;
    QString getDialogColor() const;
    QString getMenuColor() const;
    QString getTooltipColor() const;

    // ========================================================================
    // Member Variables
    // ========================================================================
    
    // State
    bool m_darkTheme;
    bool m_initialized;
    bool m_dirty;
    
    // Primary colors
    QString m_primaryColor;
    QString m_primaryLightColor;
    QString m_primaryDarkColor;
    
    // Accent colors
    QString m_accentColor;
    QString m_accentLightColor;
    QString m_accentDarkColor;
    
    // Status colors
    QString m_successColor;
    QString m_warningColor;
    QString m_errorColor;
    QString m_infoColor;
    
    // Background colors
    QString m_backgroundColor;
    QString m_backgroundLightColor;
    QString m_backgroundDarkColor;
    
    // Surface colors
    QString m_surfaceColor;
    QString m_surfaceLightColor;
    QString m_surfaceDarkColor;
    
    // Text colors
    QString m_textColor;
    QString m_textSecondaryColor;
    QString m_textHintColor;
    QString m_textDisabledColor;
    
    // Border colors
    QString m_borderColor;
    QString m_borderLightColor;
    QString m_borderDarkColor;
    
    // Shadow colors
    QString m_shadowColor;
    QString m_shadowLightColor;
    QString m_shadowDarkColor;
    
    // Component colors
    QString m_cardColor;
    QString m_dialogColor;
    QString m_menuColor;
    QString m_tooltipColor;
    
    // Custom colors
    QMap<QString, QString> m_customColors;
    
    // File management
    QString m_themeFile;
    QTimer m_autoSaveTimer;
    
    // Default color constants
    static const QString DEFAULT_PRIMARY;
    static const QString DEFAULT_ACCENT;
    static const QString DEFAULT_SUCCESS;
    static const QString DEFAULT_WARNING;
    static const QString DEFAULT_ERROR;
    static const QString DEFAULT_INFO;
};

#endif // THEME_MANAGER_H
