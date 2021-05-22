#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtDebug>
#include <QDir>
#include <QFile>
#include <QFileInfoList>
#include <QFileInfo>
#include <QFileDialog>
#include <QProgressBar>
#include <QMessageBox>
#include <QVector>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QProgressDialog>
#include <QProcess>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    QJsonObject* parseJsonFile(const QString& file);

private:
    int findBilibiliInstall(QString& );
    QStringList* searchVideo(const QString& path);
    void showVideo(const QStringList&);
    void changeItemCheckState(QListWidgetItem *item, int c);
signals:
    void got_dir(const QStringList& source_path, const QString& des_path);
    void cancle(void);
    void proccess_present(int);

private slots:
    void handle_toolButton(bool);
    void handle_toolButton2(bool);
    void handle_pushButton(bool);
    void handle_pushButton2(bool);
    void handle_pushButton3(bool);

    void seleteItem(QListWidgetItem *item);
    void rename(const QStringList& source_path, const QString& des_path);
    void handle_cancle(void);
    void show_proccess(int);
private:

    Ui::Widget *ui;

    QString _source_path;
    QString _des_path;
    QString _bi_data_path;
    QStringList _video_list;
//    QStringList _video_rename_list;

    bool _is_procceess = false;
    bool _is_cancle = false;
};
#endif // WIDGET_H
