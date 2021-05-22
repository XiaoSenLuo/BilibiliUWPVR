#include "widget.h"
#include "ui_widget.h"

#include "string.h"

//#include "windows.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    connect(ui->toolButton, SIGNAL(clicked(bool)), this, SLOT(handle_toolButton(bool)));
    connect(ui->toolButton_2, SIGNAL(clicked(bool)), this, SLOT(handle_toolButton2(bool)));
    connect(ui->pushButton, SIGNAL(clicked(bool)), this, SLOT(handle_pushButton(bool)));
    connect(ui->pushButton_2, SIGNAL(clicked(bool)), this, SLOT(handle_pushButton2(bool)));
    connect(ui->pushButton_3, SIGNAL(clicked(bool)), this, SLOT(handle_pushButton3(bool)));
    connect(ui->listWidget, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(seleteItem(QListWidgetItem *)));

    connect(this, SIGNAL(got_dir(const QStringList&, const QString&)), this, SLOT(rename(const QStringList&, const QString&)));
    connect(this, SIGNAL(cancle()), this, SLOT(handle_cancle()));
    connect(this, SIGNAL(proccess_present(int)), this, SLOT(show_proccess(int)));

    QString path;
    QStringList *list;
    if(findBilibiliInstall(path) == 0){
        qDebug() << path;
        path.append(QString::fromUtf8("/LocalCache/BilibiliDownload"));
        _bi_data_path = path;
        list = searchVideo(_bi_data_path);
        _video_list = QStringList(*list);
        showVideo(_video_list);
    }else{
        QMessageBox::warning(this, QString::fromUtf8("警告"), QString::fromUtf8("无法找到Bilibili下载目录, 请手动选择"));
    }
}




Widget::~Widget()
{
    delete ui;
}

QJsonObject *Widget::parseJsonFile(const QString &file)
{
    QFile json_file(file);
    if(json_file.open(QIODevice::ReadOnly) == false) return nullptr;
    QByteArray json_str = json_file.readAll();
    json_file.close();

    QJsonDocument json_doc = QJsonDocument::fromJson(json_str);
    return new QJsonObject(json_doc.object());
}

int Widget::findBilibiliInstall(QString& path)
{
//    WinExec("powershell.exe Get-AppPackage *forWin10 >> info.txt", SW_SHOW);
    int res = -1;
    QProcess* p = new QProcess(this);
    p->setReadChannel(QProcess::StandardOutput);
    p->setProgram("powershell.exe");
    p->setArguments(QStringList() << "Get-AppPackage 36699Atelier39.forWin10");
    p->open();
    if(!p->waitForStarted()){ return -1;}
    if(!p->waitForFinished()){return -1;}
    const QByteArray ret = p->readAllStandardOutput();
    p->close();

    path = QDir::homePath();
    path.append("/AppData/Local/Packages/");

    QString ret_str = QString(ret);

    QStringList ret_l = ret_str.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
    for(int i = 0; i < ret_l.size(); i++){
        if(ret_l.at(i).compare(QString::fromUtf8("PackageFamilyName")) == 0){
            if(ret_l.at(i+1).compare(QString::fromUtf8(":")) == 0){
                path.append(ret_l.at(i+2));
                res = 0;
                break;
            }else{
                res = -1;
            }
        }else{
            res = -1;
        }
    }
//    qDebug() << path;
    return res;
}

QStringList* Widget::searchVideo(const QString &path)
{
    QDir dir = QDir(path);
    QStringList child_list = dir.entryList(QDir::Dirs);
    child_list.removeOne(QString("."));
    child_list.removeOne(QString(".."));
    QStringList list;
    QString _path;
    for(int i = 0; i < child_list.size(); i++){
        _path = path + "/" + child_list.at(i) + "/" + child_list.at(i) + ".dvi";
        list.append(_path);
    }
    return new QStringList(list);
}

void Widget::showVideo(const QStringList &)
{
    ui->listWidget->clear();
    ui->listWidget->addItem(QString::fromUtf8("全选"));

    QJsonObject *root;
    for(int i = 0; i < _video_list.count(); i++){
        root = parseJsonFile(_video_list.at(i));
        if(root->contains(QString::fromUtf8("Title"))){
            const QString _t = root->value(QString::fromUtf8("Title")).toString();
            ui->listWidget->addItem(_t);
            delete root;
        }
    }
    int c = ui->listWidget->count();
    for(int i = 0; i < c; i++){
        ui->listWidget->item(i)->setCheckState(Qt::Unchecked);
        ui->listWidget->item(i)->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    }
}

void Widget::changeItemCheckState(QListWidgetItem *item, int c)
{
    if(c == Qt::Unchecked){
        item->setCheckState(Qt::Unchecked);
        item->setBackground(QBrush(QColor(Qt::white)));
    }else{
        item->setCheckState(Qt::Checked);
        item->setBackground(QBrush(QColor("#e5f3ff")));
    }
}


void Widget::handle_toolButton(bool b)
{
    Q_UNUSED(b);
    const QString dir = QFileDialog::getOpenFileName(this, QString::fromUtf8("打开文件"), _source_path, QString::fromUtf8("DVI 文件(*.dvi);;*.*"));
    _source_path = dir.mid(0, dir.lastIndexOf('/'));
    ui->lineEdit->setText(dir);
}

void Widget::handle_toolButton2(bool b)
{
    Q_UNUSED(b);

    const QString dir = QFileDialog::getExistingDirectory(this, QString::fromUtf8("打开目录"), _des_path,  QFileDialog::ShowDirsOnly);
    _des_path = dir;
    ui->lineEdit_2->setText(dir);
}

void Widget::handle_pushButton(bool b)
{
    Q_UNUSED(b);
    if(_is_procceess){
        _is_procceess = false;
    }
    _is_cancle = true;
}

void Widget::handle_pushButton2(bool b)
{
    Q_UNUSED(b);
    const QString source_path = ui->lineEdit->text();
    const QString des_path = ui->lineEdit_2->text();

    if(_is_procceess){
        QMessageBox::warning(this, QString::fromUtf8("警告"), QString::fromUtf8("正在处理!"));
        return;
    }
    QStringList _video_re_list;
    if(source_path.length() > 0) _video_re_list.append(source_path);
    for(int i = 1; i < ui->listWidget->count(); i++){
        if(ui->listWidget->item(i)->checkState() == Qt::Checked){
             _video_re_list.append(_video_list.at(i - 1));
        }
    }
//    qDebug() << _video_re_list;
    if(_video_re_list.length() > 0) emit got_dir(_video_re_list, des_path);
}

void Widget::handle_pushButton3(bool b)
{
    Q_UNUSED(b);
    QStringList *list;
    list = searchVideo(_bi_data_path);
    _video_list.clear();
    _video_list = QStringList(*list);
    showVideo(_video_list);
    qDebug() << QString::fromUtf8("刷新完成");
}

void Widget::seleteItem(QListWidgetItem *item)
{
    bool b = item->isSelected();
    int c = item->checkState();

    item->setSelected(!b);

    if(c != Qt::Unchecked) changeItemCheckState(item, Qt::Unchecked);
    else changeItemCheckState(item, Qt::Checked);

    if(ui->listWidget->row(item) == 0){
        for(int i = 0; i < ui->listWidget->count(); i++){
//            ui->listWidget->item(i)->setSelected(!b);
            if(c != Qt::Unchecked) changeItemCheckState(ui->listWidget->item(i), Qt::Unchecked);
            else changeItemCheckState(ui->listWidget->item(i), Qt::Checked);
        }
    }
//    qDebug() << "clicked row:" << ui->listWidget->row(item);;
}


void Widget::rename(const QStringList& source_path, const QString& des_path)
{
    if(_is_procceess) return;
    _is_procceess = true;
    _is_cancle = false;
//    qDebug() << source_path;
    if(source_path.size() == 0) return;

    QString _des_path;
    QDir des_dir;
    if(des_path.length() == 0){
        _des_path = QCoreApplication::applicationDirPath();
        des_dir = QDir(_des_path);
//        qDebug() << _des_path;
    }else{
        des_dir = QDir(des_path);
        _des_path = des_path;
    }
    if(des_dir.exists() == false){
        if(des_dir.mkpath(des_path) == false){
            QMessageBox::critical(this, QString::fromUtf8("错误!"), QString::fromUtf8("目录创建失败"));
//            qDebug() << ret;
            return;
        }
    }


    QString _source_path, _des_curr_path;
    QString parent_path;
    QDir parent_dir;
    QStringList dir_list;
    QList<QStringList> _dir_all_list;
    int _progress_index = 0;
    QJsonObject* root;
    QString aid_str;
    QString title_str;
    QString _path, _partNO, _partName, _reFileName;
    QDir _child_dir;
    QStringList _file_info_list;
    int index = 0;
    QFile *log;
    int _all_file_number = 0;

    for(int si = 0; si < source_path.size(); si++){  // 遍历所有视频
        _source_path = source_path.at(si);

//        i = _source_path.lastIndexOf('/');
        parent_path = _source_path.mid(0, _source_path.lastIndexOf('/'));
    //    qDebug() << parent_path;

        parent_dir = QDir(parent_path);

        dir_list = parent_dir.entryList(QDir::Dirs, QDir::Name);
        dir_list.removeOne(QString("."));
        dir_list.removeOne(QString(".."));

        _dir_all_list.append(dir_list);
        _all_file_number += dir_list.size();
//        dir_list.clear();
//        delete &dir_list;
    }
    qDebug() << "All Files:" << _all_file_number;
    QProgressDialog progress(QString::fromUtf8("正在复制文件..."), QString::fromUtf8("取消"), 0, 100, this);
    progress.setWindowModality(Qt::WindowModal);

    for(int si = 0; si < source_path.size(); si++){   // 开始复制
        if(progress.wasCanceled()) break;
        if(_is_cancle) break;
        if(_dir_all_list.at(si).size() == 0) break;
        _source_path = source_path.at(si);
        qDebug() << "source file:" << _source_path;

//    if(dir_list.size() == 0) return;
        root = parseJsonFile(_source_path);

        if(root->contains(QString("Aid"))){
            aid_str = root->value(QString("Aid")).toString();
            qDebug() << "Aid:" << aid_str;
        }else{
            return;
        }
        if(root->contains(QString("Title"))){
            title_str = root->value(QString("Title")).toString();
            progress.setLabelText(QString::fromUtf8("正在复制: ") + title_str);
            qDebug() << "Title:" << title_str;
        }else{
            return;
        }
        delete root;
        _des_curr_path = _des_path;
        _des_curr_path.append('/');
        _des_curr_path.append(title_str);

        des_dir = QDir(_des_curr_path);
        des_dir.mkpath(_des_curr_path);

        log = new QFile(_des_curr_path + "/" + "log.txt");
        log->open(QIODevice::ReadWrite);

        index = 0;
        dir_list = _dir_all_list.at(si);

        progress.setMaximum(dir_list.size());

        while(!_is_cancle && (index < dir_list.size())){
            if(progress.wasCanceled()) break;

            parent_path = _source_path.mid(0, _source_path.lastIndexOf('/'));
            _path = parent_path + "/" + dir_list.at(index);
            _child_dir = QDir(_path);
            _file_info_list = _child_dir.entryList(QDir::Files, QDir::Name);
    //        qDebug() << _file_info_list;

            root = parseJsonFile(_path + "/" + aid_str + ".info");

            if(root->contains(QString("PartNo"))){
                _partNO = root->value(QString("PartNo")).toString();
            }
            if(root->contains(QString("PartName"))){
                _partName = root->value(QString("PartName")).toString();
            }
            if(_file_info_list.size() > 0){
                QString tmp;
                for(int i = 0; i < _file_info_list.size(); i++){
                    tmp = _file_info_list.at(i);
                    if(tmp.contains(".info")){

                    }
                    if((tmp.contains(".mp4") || tmp.contains(".flv")) && (tmp.contains(aid_str))){
                        _reFileName = tmp;
                        _reFileName = _reFileName.replace(0, aid_str.length(), _partName);
                        if(QFile(_des_curr_path + "/" + _reFileName).exists()) QFile::remove(_des_curr_path + "/" + _reFileName);
                        QTextStream(log) << _path + "/" + tmp + " --> " + _des_curr_path + "/" + _reFileName + ":";
                        if(QFile::copy(_path + "/" + tmp, _des_curr_path + "/" + _reFileName)){
                            QTextStream(log) << QString::fromUtf8("成功\n");
                        }else{
                            QTextStream(log) << QString::fromUtf8("失败\n");
                        }
                    }
                }
            }
            index += 1;
            _progress_index += 1;
            progress.setValue(index);
            emit proccess_present(int((float(_progress_index) / _all_file_number) * 100));
        }
        log->close();
        delete log;
    //    delete &dir_list;
    //    delete &_file_info_list;
    }
    _is_procceess = false;
    _is_cancle = false;
}

void Widget::handle_cancle()
{
    _is_cancle = true;
}

void Widget::show_proccess(int i)
{
    ui->progressBar->setValue(i);
}




