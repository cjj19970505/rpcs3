#pragma once

#include "game_list.h"
#include "custom_dock_widget.h"
#include "gui_save.h"
#include "shortcut_utils.h"
#include "Utilities/lockless.h"
#include "Emu/System.h"

#include <QMainWindow>
#include <QToolBar>
#include <QStackedWidget>
#include <QSet>
#include <QTableWidgetItem>
#include <QFutureWatcher>

#include <memory>

class game_list_grid;
class gui_settings;
class emu_settings;
class persistent_settings;

class game_list_frame : public custom_dock_widget
{
	Q_OBJECT

public:
	explicit game_list_frame(std::shared_ptr<gui_settings> gui_settings, std::shared_ptr<emu_settings> emu_settings, std::shared_ptr<persistent_settings> persistent_settings, QWidget* parent = nullptr);
	~game_list_frame();

	/** Fix columns with width smaller than the minimal section size */
	void FixNarrowColumns() const;

	/** Resizes the columns to their contents and adds a small spacing */
	void ResizeColumnsToContents(int spacing = 20) const;

	/** Refresh the gamelist with/without loading game data from files. Public so that main frame can refresh after vfs or install */
	void Refresh(const bool from_drive = false, const bool scroll_after = true);

	/** Adds/removes categories that should be shown on gamelist. Public so that main frame menu actions can apply them */
	void ToggleCategoryFilter(const QStringList& categories, bool show);

	/** Loads from settings. Public so that main frame can easily reset these settings if needed. */
	void LoadSettings();

	/** Saves settings. Public so that main frame can save this when a caching of column widths is needed for settings backup */
	void SaveSettings();

	/** Resize Gamelist Icons to size given by slider position */
	void ResizeIcons(const int& slider_pos);

	/** Repaint Gamelist Icons with new background color */
	void RepaintIcons(const bool& from_settings = false);

	void SetShowHidden(bool show);

	game_compatibility* GetGameCompatibility() const { return m_game_compat; }

	const QList<game_info>& GetGameInfo() const;

	// Returns the visible version string in the game list
	static std::string GetGameVersion(const game_info& game);

	void CreateShortcuts(const game_info& gameinfo, const std::set<gui::utils::shortcut_location>& locations);

public Q_SLOTS:
	void BatchCreatePPUCaches();
	void BatchRemovePPUCaches();
	void BatchRemoveSPUCaches();
	void BatchRemoveCustomConfigurations();
	void BatchRemoveCustomPadConfigurations();
	void BatchRemoveShaderCaches();
	void SetListMode(const bool& is_list);
	void SetSearchText(const QString& text);
	void SetShowCompatibilityInGrid(bool show);
	void SetShowCustomIcons(bool show);
	void SetPlayHoverGifs(bool play);

private Q_SLOTS:
	void OnRefreshFinished();
	void OnRepaintFinished();
	void OnCompatFinished();
	void OnColClicked(int col);
	void ShowContextMenu(const QPoint &pos);
	void doubleClickedSlot(QTableWidgetItem *item);
	void ItemSelectionChangedSlot();
Q_SIGNALS:
	void GameListFrameClosed();
	void NotifyGameSelection(const game_info& game);
	void RequestBoot(const game_info& game, cfg_mode config_mode = cfg_mode::custom, const std::string& config_path = "", const std::string& savestate = "");
	void RequestIconSizeChange(const int& val);
	void NotifyEmuSettingsChange();
protected:
	/** Override inherited method from Qt to allow signalling when close happened.*/
	void closeEvent(QCloseEvent* event) override;
	void resizeEvent(QResizeEvent *event) override;
	bool eventFilter(QObject *object, QEvent *event) override;
private:
	QPixmap PaintedPixmap(const QPixmap& icon, bool paint_config_icon = false, bool paint_pad_config_icon = false, const QColor& color = QColor()) const;
	QColor getGridCompatibilityColor(const QString& string) const;

	/** Sets the custom config icon. Only call this for list title items. */
	void SetCustomConfigIcon(QTableWidgetItem* title_item, const game_info& game);
	void ShowCustomConfigIcon(const game_info& game);
	void PopulateGameList();
	void PopulateGameGrid(int maxCols, const QSize& image_size, const QColor& image_color);
	bool IsEntryVisible(const game_info& game);
	void SortGameList() const;
	bool SearchMatchesApp(const QString& name, const QString& serial) const;

	bool RemoveCustomConfiguration(const std::string& title_id, const game_info& game = nullptr, bool is_interactive = false);
	bool RemoveCustomPadConfiguration(const std::string& title_id, const game_info& game = nullptr, bool is_interactive = false);
	bool RemoveShadersCache(const std::string& base_dir, bool is_interactive = false);
	bool RemovePPUCache(const std::string& base_dir, bool is_interactive = false);
	bool RemoveSPUCache(const std::string& base_dir, bool is_interactive = false);
	static bool CreatePPUCache(const std::string& path, const std::string& serial = {});
	static bool CreatePPUCache(const game_info& game);

	QString GetLastPlayedBySerial(const QString& serial) const;
	static std::string GetCacheDirBySerial(const std::string& serial);
	static std::string GetDataDirBySerial(const std::string& serial);
	std::string CurrentSelectionPath();
	static std::string GetStringFromU32(const u32& key, const std::map<u32, QString>& map, bool combined = false);

	game_info GetGameInfoByMode(const QTableWidgetItem* item) const;
	static game_info GetGameInfoFromItem(const QTableWidgetItem* item);

	// Which widget we are displaying depends on if we are in grid or list mode.
	QMainWindow* m_game_dock = nullptr;
	QStackedWidget* m_central_widget = nullptr;

	// Game Grid
	game_list_grid* m_game_grid = nullptr;

	// Game List
	game_list* m_game_list = nullptr;
	game_compatibility* m_game_compat = nullptr;
	QList<QAction*> m_columnActs;
	Qt::SortOrder m_col_sort_order;
	int m_sort_column;
	QMap<QString, QString> m_notes;
	QMap<QString, QString> m_titles;

	// Categories
	QStringList m_category_filters;

	// List Mode
	bool m_is_list_layout = true;
	bool m_old_layout_is_list = true;

	// Data
	std::shared_ptr<gui_settings> m_gui_settings;
	std::shared_ptr<emu_settings> m_emu_settings;
	std::shared_ptr<persistent_settings> m_persistent_settings;
	QList<game_info> m_game_data;
	std::vector<std::string> m_path_list;
	QSet<QString> m_serials;
	QMutex m_mutex_cat;
	lf_queue<game_info> m_games;
	QFutureWatcher<void> m_refresh_watcher;
	QFutureWatcher<movie_item*> m_repaint_watcher;
	QSet<QString> m_hidden_list;
	bool m_show_hidden{false};

	// Search
	QString m_search_text;

	// Icon Size
	int m_icon_size_index = 0;

	// Icons
	QColor m_icon_color;
	QSize m_icon_size;
	qreal m_margin_factor;
	qreal m_text_factor;
	bool m_draw_compat_status_to_grid = false;
	bool m_show_custom_icons = true;
	bool m_play_hover_movies = true;
};
