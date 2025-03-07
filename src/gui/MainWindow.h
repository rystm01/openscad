#pragma once

#include <ctime>
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

#include <QAction>
#include <QCloseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QEvent>
#include <QFile>
#include <QLabel>
#include <QList>
#include <QMap>
#include <QObject>
#include <QPoint>
#include <QString>
#include <QStringList>
#include <QTemporaryFile>
#include <QTimer>
#include <QUrl>
#include <QWidget>
#include <QMainWindow>
#include <QElapsedTimer>
#include <QIcon>
#include <QIODevice>
#include <QMutex>
#include <QSoundEffect>
#include <QTime>
#include <QSignalMapper>

#include "gui/Editor.h"
#include "geometry/Geometry.h"
#include "io/export.h"
#include "gui/Measurement.h"
#include "RenderStatistic.h"
#include "gui/TabManager.h"
#include "core/Tree.h"
#include "gui/UIUtils.h"
#include "gui/qtgettext.h" // IWYU pragma: keep
#include "gui/qt-obsolete.h" // IWYU pragma: keep
#include "ui_MainWindow.h"

#ifdef STATIC_QT_SVG_PLUGIN
#include <QtPlugin>
Q_IMPORT_PLUGIN(QSvgPlugin)
#endif

class BuiltinContext;
class CGALWorker;
class CSGNode;
class CSGProducts;
class FontListDialog;
class LibraryInfoDialog;
class Preferences;
class ProgressWidget;
class ThrownTogetherRenderer;

class MainWindow : public QMainWindow, public Ui::MainWindow, public InputEventHandler
{
  Q_OBJECT

public:
  Preferences *prefs;

  QTimer *consoleUpdater;

  bool isPreview;

  QTimer *autoReloadTimer;
  QTimer *waitAfterReloadTimer;
  RenderStatistic renderStatistic;

  SourceFile *rootFile; // Result of parsing
  SourceFile *parsedFile; // Last parse for include list
  std::shared_ptr<AbstractNode> absoluteRootNode; // Result of tree evaluation
  std::shared_ptr<AbstractNode> rootNode; // Root if the root modifier (!) is used
#ifdef ENABLE_PYTHON
  bool python_active;
  std::string trusted_edit_document_name;
  std::string untrusted_edit_document_name;
  bool trust_python_file(const std::string& file, const std::string& content);
#endif
  Tree tree;
  EditorInterface *activeEditor;
  TabManager *tabManager;

  std::shared_ptr<const Geometry> rootGeom;
  std::shared_ptr<Renderer> cgalRenderer;
#ifdef ENABLE_OPENCSG
  std::shared_ptr<Renderer> opencsgRenderer;
#endif
  std::shared_ptr<Renderer> thrownTogetherRenderer;

  QString lastCompiledDoc;

  QAction *actionRecentFile[UIUtils::maxRecentFiles];
  QMap<QString, QString> knownFileExtensions;

  QLabel *versionLabel;
  QWidget *editorDockTitleWidget;
  QWidget *consoleDockTitleWidget;
  QWidget *parameterDockTitleWidget;
  QWidget *errorLogDockTitleWidget;
  QWidget *animateDockTitleWidget;
  QWidget *viewportControlTitleWidget;
  QWidget *fontListDockTitleWidget;

  Measurement meas;

  int compileErrors;
  int compileWarnings;

  MainWindow(const QStringList& filenames);
  ~MainWindow() override;

private:
  volatile bool isClosing = false;
  void consoleOutputRaw(const QString& msg);
  void clearAllSelectionIndicators();
  void setSelectionIndicatorStatus(int nodeIndex, EditorSelectionIndicatorStatus status);

protected:
  void closeEvent(QCloseEvent *event) override;

private slots:
  void updateUndockMode(bool undockMode);
  void updateReorderMode(bool reorderMode);
  void setFont(const QString& family, uint size);
  void setColorScheme(const QString& cs);
  void showProgress();
  void openCSGSettingsChanged();
  void consoleOutput(const Message& msgObj);
  void setSelection(int index);
  void onHoveredObjectInSelectionMenu();
  void measureFinished();
  void errorLogOutput(const Message& log_msg);

public:
  static void consoleOutput(const Message& msgObj, void *userdata);
  static void errorLogOutput(const Message& log_msg, void *userdata);
  static void noOutputConsole(const Message&, void *) {} // /dev/null
  static void noOutputErrorLog(const Message&, void *) {} // /dev/null

  bool fileChangedOnDisk();
  void parseTopLevelDocument();
  void exceptionCleanup();
  void setLastFocus(QWidget *widget);
  void UnknownExceptionCleanup(std::string msg = "");

private:
  void setRenderVariables(ContextHandle<BuiltinContext>& context);
  void updateCompileResult();
  void compile(bool reload, bool forcedone = false);
  void compileCSG();
  bool checkEditorModified();
  QString dumpCSGTree(const std::shared_ptr<AbstractNode>& root);

  void loadViewSettings();
  void loadDesignSettings();
  void prepareCompile(const char *afterCompileSlot, bool procevents, bool preview);
  void updateWindowSettings(bool console, bool editor, bool customizer, bool errorLog, bool editorToolbar, bool viewToolbar, bool animate, bool fontList, bool ViewportControlWidget);
  void saveBackup();
  void writeBackup(QFile *file);
  void show_examples();
  void setDockWidgetTitle(QDockWidget *dockWidget, QString prefix, bool topLevel);
  void addKeyboardShortCut(const QList<QAction *>& actions);
  void updateStatusBar(ProgressWidget *progressWidget);
  void activateWindow(int);

  LibraryInfoDialog *libraryInfoDialog{nullptr};
  FontListDialog *fontListDialog{nullptr};
  QSignalMapper *exportFormatMapper;

public slots:
  void updateExportActions();
  void updateRecentFiles(const QString& FileSavedOrOpened);
  void updateRecentFileActions();
  void handleFileDrop(const QUrl& url);

private slots:
  void actionOpen();
  void actionNewWindow();
  void actionOpenWindow();
  void actionOpenRecent();
  void actionSendChat();
  void actionOpenExample();
  void clearRecentFiles();
  void actionSave();
  void actionSaveAs();
  void actionRevokeTrustedFiles();
  void actionSaveACopy();
  void actionReload();
  void actionShowLibraryFolder();
  void convertTabsToSpaces();
  void copyText();

  void instantiateRoot();
  void compileDone(bool didchange);
  void compileEnded();
  void changeParameterWidget();

private slots:
  void copyViewportTranslation();
  void copyViewportRotation();
  void copyViewportDistance();
  void copyViewportFov();
  void preferences();
  void hideEditorToolbar();
  void hide3DViewToolbar();
  void showLink(const QString&);
  void showEditor();
  void hideEditor();
  void showConsole();
  void hideConsole();
  void showErrorLog();
  void hideErrorLog();
  void showViewportControl();
  void hideViewportControl();
  void showParameters();
  void hideParameters();
  void showAnimate();
  void hideAnimate();
  void showFontList();
  void hideFontList();
  void on_windowActionSelectEditor_triggered();
  void on_windowActionSelectConsole_triggered();
  void on_windowActionSelectCustomizer_triggered();
  void on_windowActionSelectErrorLog_triggered();
  void on_windowActionSelectAnimate_triggered();
  void on_windowActionSelectFontList_triggered();
  void on_windowActionSelectViewportControl_triggered();
  void on_windowActionNextWindow_triggered();
  void on_windowActionPreviousWindow_triggered();
  void on_editActionInsertTemplate_triggered();
  void on_editActionFoldAll_triggered();

public slots:
  void hideFind();
  void showFind();
  void showFindAndReplace();

private slots:
  void selectFindType(int);
  void findString(const QString&);
  void findNext();
  void findPrev();
  void useSelectionForFind();
  void replace();
  void replaceAll();

  // Mac OSX FindBuffer support
  void findBufferChanged();
  void updateFindBuffer(const QString&);
  bool event(QEvent *event) override;
protected:
  bool eventFilter(QObject *obj, QEvent *event) override;

public slots:
  void actionRenderPreview();
private slots:
  void csgRender();
  void csgReloadRender();
  void action3DPrint();
  void sendToExternalTool(class ExternalToolInterface& externalToolService);
  void actionRender();
  void actionRenderDone(const std::shared_ptr<const Geometry>&);
  void cgalRender();
  void actionMeasureDistance();
  void actionMeasureAngle();
  void actionCheckValidity();
  void actionDisplayAST();
  void actionDisplayCSGTree();
  void actionDisplayCSGProducts();
  bool canExport(unsigned int dim);
  void actionExport(unsigned int dim, ExportInfo& exportInfo);
  void actionExportFileFormat(int fmt);
  void actionCopyViewport();
  void actionFlushCaches();

public:
  void viewModeActionsUncheck();
  void setCurrentOutput();
  void clearCurrentOutput();
  void hideCurrentOutput();
  bool isEmpty();

  void onAxisChanged(InputEventAxisChanged *event) override;
  void onButtonChanged(InputEventButtonChanged *event) override;

  void onTranslateEvent(InputEventTranslate *event) override;
  void onRotateEvent(InputEventRotate *event) override;
  void onRotate2Event(InputEventRotate2 *event) override;
  void onActionEvent(InputEventAction *event) override;
  void onZoomEvent(InputEventZoom *event) override;

  void changedTopLevelConsole(bool);
  void changedTopLevelErrorLog(bool);
  void changedTopLevelAnimate(bool);
  void changedTopLevelFontList(bool);
  void changedTopLevelViewportControl(bool);

  QList<double> getTranslation() const;
  QList<double> getRotation() const;
  std::unordered_map<FileFormat, QAction *> exportMap;

public slots:
  void actionReloadRenderPreview();
  void on_editorDock_visibilityChanged(bool);
  void on_consoleDock_visibilityChanged(bool);
  void on_parameterDock_visibilityChanged(bool);
  void on_errorLogDock_visibilityChanged(bool);
  void on_animateDock_visibilityChanged(bool);
  void on_fontListDock_visibilityChanged(bool);
  void on_viewportControlDock_visibilityChanged(bool);
  void on_toolButtonCompileResultClose_clicked();
  void consoleTopLevelChanged(bool);
  void parameterTopLevelChanged(bool);
  void errorLogTopLevelChanged(bool);
  void animateTopLevelChanged(bool);
  void fontListTopLevelChanged(bool);
  void viewportControlTopLevelChanged(bool);
  void processEvents();
  void jumpToLine(int, int);
  void openFileFromPath(const QString&, int);

  void viewModeRender();
#ifdef ENABLE_OPENCSG
  void viewModePreview();
#endif
  void viewModeThrownTogether();
  void viewModeShowEdges();
  void viewModeShowAxes();
  void viewModeShowCrosshairs();
  void viewModeShowScaleProportional();
  void viewAngleTop();
  void viewAngleBottom();
  void viewAngleLeft();
  void viewAngleRight();
  void viewAngleFront();
  void viewAngleBack();
  void viewAngleDiagonal();
  void viewCenter();
  void viewPerspective();
  void viewOrthogonal();
  void viewTogglePerspective();
  void viewResetView();
  void viewAll();
  void editorContentChanged();
  void leftClick(QPoint coordinate);
  void rightClick(QPoint coordinate);
  void dragEnterEvent(QDragEnterEvent *event) override;
  void dropEvent(QDropEvent *event) override;
  void helpAbout();
  void helpHomepage();
  void helpManual();
  void helpOfflineManual();
  void helpCheatSheet();
  void helpOfflineCheatSheet();
  void helpLibrary();
  void helpFontInfo();
  void checkAutoReload();
  void waitAfterReload();
  void autoReloadSet(bool);

private:
  bool network_progress_func(const double permille);
  static void report_func(const std::shared_ptr<const AbstractNode>&, void *vp, int mark);
  static bool undockMode;
  static bool reorderMode;
  static const int tabStopWidth;
  static QElapsedTimer *progressThrottle;
  QWidget *lastFocus; // keep track of active copyable widget (Editor|Console) for global menu action Edit->Copy

  std::shared_ptr<CSGNode> csgRoot; // Result of the CSGTreeEvaluator
  std::shared_ptr<CSGNode> normalizedRoot; // Normalized CSG tree
  std::shared_ptr<CSGProducts> rootProduct;
  std::shared_ptr<CSGProducts> highlightsProducts;
  std::shared_ptr<CSGProducts> backgroundProducts;
  int currentlySelectedObject {-1};

  char const *afterCompileSlot;
  bool procevents{false};
  QTemporaryFile *tempFile{nullptr};
  ProgressWidget *progresswidget{nullptr};
  CGALWorker *cgalworker;
  QMutex consolemutex;
  EditorInterface *renderedEditor; // stores pointer to editor which has been most recently rendered
  time_t includesMTime{0}; // latest include mod time
  time_t depsMTime{0}; // latest dependency mod time
  std::unordered_map<QString, QString> exportPaths; // for each file type, where it was exported to last
  QString exportPath(const QString& suffix); // look up the last export path and generate one if not found
  int lastParserErrorPos{-1}; // last highlighted error position
  int tabCount = 0;
  ExportPdfPaperSize sizeString2Enum(const QString& current);
  ExportPdfPaperOrientation orientationsString2Enum(const QString& current);

  QSoundEffect *renderCompleteSoundEffect;
  std::vector<std::unique_ptr<QTemporaryFile>> allTempFiles;

signals:
  void highlightError(int);
  void unhighlightLastError();
};

class GuiLocker
{
public:
  GuiLocker() {
    GuiLocker::lock();
  }
  ~GuiLocker() {
    GuiLocker::unlock();
  }
  static bool isLocked() { return guiLocked > 0; }
  static void lock() {
    guiLocked++;
  }
  static void unlock() {
    guiLocked--;
  }

private:
  static unsigned int guiLocked;
};
