#include "mainwindow.h"
#include <QApplication>
#include "qtvisuals.h"


class VISUALS * VisualObj;
void T_ReceiveMsgs(struct AP_INFO*);
void T_Receiver();


/**
	Main function of Qt-based visualization window 
*/
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    VisualObj = new class VISUALS();
	/* Read AP details from XML config file */
    std::map<std::string, struct AP_INFO*> AP_Details = VisualObj->GetAPDetails();
    std::vector<std::thread *> z;
	/* Create message receiver thread for each AP */
    for (auto it = AP_Details.begin(); it != AP_Details.end(); it++) {
        std::cout << "hello " << it->first << std::endl;
        z.push_back(new std::thread(T_ReceiveMsgs, it->second));
    }
    z.push_back(new std::thread(T_Receiver));
    return a.exec();
}
