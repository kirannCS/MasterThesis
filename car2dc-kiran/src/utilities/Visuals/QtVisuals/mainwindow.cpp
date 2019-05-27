#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qtvisuals.h"
extern class VISUALS *VisualObj;
extern bool EnableRefresh;
extern bool ExternalRefreshQT;
extern bool EnableClusterInfoDsiplay;
std::vector<std::thread *> ListWidgets;
void T_DisplayClusterInfo(QListWidget *DisplayClusterInfo);

/**
	Displays existing cars and APs in the List of current Nodes listing box
*/
 
void T_DisplayNodes(QListWidget *DisplayExistingNodes, QLineEdit *TotalCarsLE) {
    while(true) {
	/* Checks EnableRefresh - If new cluster status information is received 
		ExternalRefreshQT - If pause button is not clicked */
        if(EnableRefresh && ExternalRefreshQT) {
            while(DisplayExistingNodes->count() > 0) {
                DisplayExistingNodes->takeItem(0);
            }

            for(auto it = VisualObj->ListOfCars.begin(); it != VisualObj->ListOfCars.end(); it++) {
                std::string APInfo = "AP -> " + it->first;
                DisplayExistingNodes->addItem(APInfo.c_str());
            }
            for(auto it = VisualObj->ListOfCars.begin(); it != VisualObj->ListOfCars.end(); it++) {
                struct CAR_INFO *CarDetails = it->second;
                for (int i = 0; i < VisualObj->NumOfCarsExisting[it->first]; i++) {
                    std::string Car = "Car -> " + CarDetails[i].ID;
                    DisplayExistingNodes->addItem(Car.c_str());
                }
            }
            int count = 0;
            for(auto it = VisualObj->NumOfCarsExisting.begin(); it != VisualObj->NumOfCarsExisting.end(); it++) {
                count += it->second;
            }
            TotalCarsLE->setText(std::to_string(count).c_str());
            EnableRefresh = false;
		/* Refresh every 2 seconds */
            sleep(2);
        }
    }
}



/**
	Qt Mainwindow widget constructor 
*/
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ListWidgets.push_back(new std::thread(T_DisplayNodes, ui->listWidget, ui->lineEdit_2));
    ListWidgets.push_back(new std::thread(T_DisplayClusterInfo, ui->listWidget_2));

}

/**
	Display cluster status information in Microcloud Details listing widget 
*/

void T_DisplayClusterInfo(QListWidget *DisplayClusterInfo) {
    while(true) {
	/* Checks EnableClusterInfoDsiplay - If new cluster status information is received 
		ExternalRefreshQT - If pause button is not clicked */
        if(EnableClusterInfoDsiplay && ExternalRefreshQT) {
		/* delete existing details in DisplayClusterInfo */
            while(DisplayClusterInfo->count() > 0) {
                DisplayClusterInfo->takeItem(0);
            }
            for(auto it = VisualObj->MC_Details.begin(); it != VisualObj->MC_Details.end(); it++) {
                struct MC_INFO *MCDetails = it->second;

		/* Add Micro Cloud details */
                std::string DisplayStr = "\n\nID = " + MCDetails->ID + "\nPosition = (" + std::to_string(MCDetails->PosX) +"," + std::to_string(MCDetails->PosY) +")\nManager = " + MCDetails->Manager;
                DisplayStr += "\nRadius = " + std::to_string(MCDetails->Radius) ;

		/* Add CH and CMs details */
                std::string CM = "\nCluster Members = ";
                std::string CH = "\nCluster Head = ";
                for (auto jt = VisualObj->ListOfCars.begin(); jt != VisualObj->ListOfCars.end(); jt++) {
                       struct CAR_INFO *CarsList = jt->second;
                       for (int i = 0; i < VisualObj->NumOfCarsExisting[jt->first]; i++) {
                           if(VisualObj->Car_vs_MC.find(CarsList[i].ID) != VisualObj->Car_vs_MC.end()) {
                               if(CarsList[i].State == "CH" && VisualObj->Car_vs_MC[CarsList[i].ID] == it->first) {
                                   CH += CarsList[i].ID + ",";
                               } else if(CarsList[i].State == "CM" && VisualObj->Car_vs_MC[CarsList[i].ID] == it->first) {
                                   CM += CarsList[i].ID + ",";
                               }
                           }

                       }

                }

		/* Add applications (data collection and aggregation and task distribution details */
                long CountInBytes = 0;
                long PrevIntervalCount = 0;
                if(VisualObj->DataCount.find(it->first) != VisualObj->DataCount.end()) {
                    CountInBytes = VisualObj->DataCount[it->first]->DataSent;
                    PrevIntervalCount = VisualObj->DataCount[it->first]->DataCountPreviousInterval;

                }
                DisplayStr += "\nTotal Data Collected = " + std::to_string(CountInBytes) + "\nData in Last Interval = " + std::to_string(PrevIntervalCount);
                DisplayStr += CH + CM;
                DisplayClusterInfo->addItem(DisplayStr.c_str());

            }
            EnableClusterInfoDsiplay = false;
        }
        sleep(1);
    }
}


/** 
	Qt mainwindow widget destructor 
*/

MainWindow::~MainWindow()
{
    delete ui;
}


/** Handles 'Pause' button 
	ExternalRefreshQt - true - do not refresh listing windows 
*/

void MainWindow::on_pushButton_clicked()
{
    qDebug() << "ButtonClicked\n";
    if(ExternalRefreshQT == false)
        ExternalRefreshQT = true;
    else {
        ExternalRefreshQT = false;
    }
}


/**
	Handles button 'FindNode'
	Reads the ID entered, if ID found in existing cars or AP displays the details
*/

void MainWindow::on_pushButton_2_clicked()
{
	/* Erase the widget content */
    while(ui->listWidget_3->count() > 0) {
        ui->listWidget_3->takeItem(0);
    }
	/* Initially assumed car or ap ID do not exist */
    bool IDFound = false;
    QString ID =  ui->lineEdit->text(); //ID of the car to be searched

	/* search for the ID in cars list */
    if(ID != "") {
        for (auto jt = VisualObj->ListOfCars.begin(); jt != VisualObj->ListOfCars.end(); jt++) {
               struct CAR_INFO *CarsList = jt->second;
               for (int i = 0; i < VisualObj->NumOfCarsExisting[jt->first]; i++) {
                   QString CarID = CarsList[i].ID.c_str();
                   if(CarID == ID) {
                       IDFound = true;
                       std::string DisplayStr = "ID = " + CarsList[i].ID + "\nPosition = " + std::to_string(CarsList[i].X) + "," + std::to_string(CarsList[i].Y) + "\n";
                       DisplayStr += "Speed = " + std::to_string(CarsList[i].Speed) + "\n" + "Direction = " + std::to_string(CarsList[i].Dir) + "\nState = " + CarsList[i].State + "\nCurrent AP = " + CarsList[i].Manager;
                       if(VisualObj->Car_vs_MC.find(CarsList[i].ID) != VisualObj->Car_vs_MC.end() && (CarsList[i].State == "CH" || CarsList[i].State == "CM")) {
                           DisplayStr += "\nMicroCloud = " + VisualObj->Car_vs_MC[CarsList[i].ID];
                       }
                       long CountInBytes = 0;
                       long PrevIntervalCount = 0;
                       if(VisualObj->DataCount.find(CarsList[i].ID) != VisualObj->DataCount.end()) {
                           CountInBytes = VisualObj->DataCount[CarsList[i].ID]->DataSent;
                           PrevIntervalCount = VisualObj->DataCount[CarsList[i].ID]->DataCountPreviousInterval;

                       }
                       DisplayStr += "\nData Sent = " + std::to_string(CountInBytes) + "\nData Sent in Last Interval = " + std::to_string(PrevIntervalCount);
                       if(VisualObj->TaskInfo.find(CarsList[i].ID) != VisualObj->TaskInfo.end()) {
                           auto DisplayTaskInfo = VisualObj->TaskInfo[CarsList[i].ID];
                           DisplayStr += "\nTotal Subtasks Assigned = " + std::to_string(DisplayTaskInfo->TotalSubTasks) + "\nTotal Subtasks Results Received = " + std::to_string(DisplayTaskInfo->TotalSubTasksResults);
                           DisplayStr += "\nTotal Subtasks Computed = " + std::to_string(DisplayTaskInfo->TotalSubTasksComputed);
                       }
			/* Display details */
                       ui->listWidget_3->addItem(DisplayStr.c_str());
                       break;
                   }
               }
        }

	/* search for the ID in AP list */
        if (!IDFound) {
               for (auto it = VisualObj->AP_Details.begin(); it != VisualObj->AP_Details.end(); it++) {
                   QString APID = it->first.c_str();
                   if(APID == ID) {
                       struct AP_INFO* APObj = it->second;
                       std::string DisplayStr = "Type = AP\nID = " + APObj->ID + "\nPosition = (" + std::to_string(APObj->PosX) + "," + std::to_string(APObj->PosY) +")";
                       DisplayStr += "\nManages = ";
                       for(auto it = VisualObj->MC_Details.begin(); it!=VisualObj->MC_Details.end(); it++) {
                           struct MC_INFO *MCInfo = it->second;
                           if(MCInfo->Manager.c_str() == ID) {
                               DisplayStr += MCInfo->ID +",";
                           }
                       }
			/* Display details */
                       ui->listWidget_3->addItem(DisplayStr.c_str());
                       IDFound = true;
                   }
               }
        }

	/* If ID not found in both cars list and AP list */
        if(!IDFound) {
            ui->listWidget_3->addItem("ID NOT FOUND");
        }
    } else {
        ui->listWidget_3->addItem("Kindly Enter ID");
    }
}
/** 
	Handles OpenGL window. When clicked pen OpenGL-based window
*/
void MainWindow::on_pushButton_3_clicked()
{
    system("pwd");
    system("python ../OpenGL.py");
}
