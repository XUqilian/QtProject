#include <core/GlobalConfig.h>

void GlobalConfig::RegisterType()
{

}

void GlobalConfig::Init()
{
    setManager = new SettingsManager(this);

    // 最后初始化页面管理器 因为页面可能依赖其它组件
    viewManager = new ViewManager(this);
    connect(viewManager,&ViewManager::doResult,this,[&](resultST ret){
        if(!ret.success)
        {
            qDebug() << "ViewManager failed because : " + ret.message;
            QCoreApplication::exit(-1);
        }
    });
}
