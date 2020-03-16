/**
  @author: Kang Lin<kl222@126.com>
  */

#include "FrmPara.h"
#include "ui_FrmPara.h"
#include "Log.h"
#include "FactoryFace.h"
#include "DelegateParamter.h"
#include <QDebug>
#include <QTreeView>
#include <QMetaObject>
#include <QMetaProperty>
#include <QStandardItem>

CFrmPara::CFrmPara(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CFrmPara)
{
    ui->setupUi(this);
    ui->treeView->setModel(&m_Model);
    ui->treeView->setItemDelegateForColumn(1, new CDelegateParamter(ui->treeView));
    bool check = connect(&m_Model, SIGNAL(itemChanged(QStandardItem *)),
                         this, SLOT(slotItemChanged(QStandardItem*)));
    Q_ASSERT(check);
    slotUpdateParamter();
}

CFrmPara::~CFrmPara()
{
    delete ui;
}

int CFrmPara::slotUpdateParamter(QAction *pAction)
{
    Q_UNUSED(pAction)
    m_Model.clear();
    //qDebug() << "CFrmPara::slotUpdateParamter";
    m_Model.setHorizontalHeaderLabels(QStringList() << tr("Property") << tr("Value"));
    
    LoadObject(CFactoryFace::Instance()->GetDector());
    LoadObject(CFactoryFace::Instance()->GetLandmarker());
    LoadObject(CFactoryFace::Instance()->GetRecognizer());
    LoadObject(CFactoryFace::Instance()->GetTracker());
    LoadObject(CFactoryFace::Instance()->GetFaceTools());
    
    return 0;
}

int CFrmPara::LoadObject(QObject *pObject)
{
    int nRet = 0;
    if(!pObject) return -1;

    const QMetaObject *pMO = pObject->metaObject();

    QStandardItem *pClass = new QStandardItem(pMO->className());
    pClass->setEditable(false);
    m_Model.appendRow(pClass);
    int nCount = pMO->propertyCount();

    for(int i = 0; i < nCount; i++)
    {
        QMetaProperty p = pMO->property(i);
        if(!p.isValid())
        {
            continue;
        }

        QString szName(p.name());
        QStandardItem* pItem = new QStandardItem(szName);
        pItem->setEditable(false);
        pClass->appendRow(pItem);

        QStandardItem* pValue = new QStandardItem();
        pClass->setChild(pItem->index().row(), 1,  pValue);
        QVariant value = p.read(pObject);
        pValue->setData(value, Qt::EditRole);
        pValue->setEditable(p.isWritable());

        pValue->setData(szName, CDelegateParamter::ROLE_PROPERTY_NAME);

        QVariant obj;
        obj.setValue(pObject);
        pValue->setData(obj, CDelegateParamter::ROLE_OBJECT);

        pValue->setData(CDelegateParamter::TYPE_OTHER,
                        CDelegateParamter::ROLE_PROPERTY_TYPE);
        if(p.isEnumType() || p.isFlagType())
        {
            int curValue = 0;
            QString szEnum;
            QMetaEnum em = p.enumerator();
            for(int j = 0; j < em.keyCount(); j++)
            {
                QString szVal;
                szEnum += em.key(j) + QString("=")
                        + szVal.setNum(em.value(j)) + ";";
                if(em.value(i) == value.toInt())
                    curValue = i;
            }

            pValue->setData(em.key(curValue), Qt::DisplayRole);
            pValue->setData(szEnum, CDelegateParamter::ROLE_PROPERTY_VALUE);
            pValue->setData(CDelegateParamter::TYPE_ENUM,
                            CDelegateParamter::ROLE_PROPERTY_TYPE);
            continue;
        }
        
        QRegExp rx(".*[Pp]ath.*");
        if(rx.exactMatch(szName))
        {
            pValue->setData(CDelegateParamter::TYPE_DIRECTORY,
                            CDelegateParamter::ROLE_PROPERTY_TYPE);
            continue;
        }
        
        rx = QRegExp(".*[Ff]ile.*");
        if(rx.exactMatch(szName))
        {
            pValue->setData(CDelegateParamter::TYPE_FILE,
                            CDelegateParamter::ROLE_PROPERTY_TYPE);
            continue;
        }
    }

    return nRet;
}

void CFrmPara::slotItemChanged(QStandardItem* item)
{
    QObject* pObject = item->data(CDelegateParamter::ROLE_OBJECT)
            .value<QObject*>();
    if(pObject)
        pObject->setProperty(
                item->data(CDelegateParamter::ROLE_PROPERTY_NAME)
                    .toString().toStdString().c_str(),
                item->data(Qt::EditRole));
}