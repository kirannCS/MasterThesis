//
// Created by kiran.narayanaswamy on 12/10/18.
//

#include "Visuals.h"
float area(int x1, int y1, int x2, int y2, int x3, int y3);
bool check(int, int, int, int, int, int, int, int, int x, int y);


TRAFFIC_VISUALS *VisualObj;

long W_Size_X;
long W_Size_Y;


/* Initialize OpenGL Graphics */
void initGL() {
    // Set "clearing" or background color
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f); // Black and opaque
}

/**
	The function populates Map VisualObj->Car_vs_MC[]
	This represents all the cluster members and thier cluster IDs
*/
void TRAFFIC_VISUALS::CarVsMCDetails(CLUSTERINFO *CarsInfo) {
	/* CarsInfo->cmlist() returns string */
    std::string CarsInfoStr = CarsInfo->cmlist();
    std::stringstream ss(CarsInfoStr);
	/* Extract from string to identify Car IDs and Cluster IDs */
    while( ss.good() ) {
        std::string substr;
        getline(ss, substr, ',');
        if (!substr.empty()) {
            VisualObj->Car_vs_MC[substr] = CarsInfo->clusterid();
        }
    }
}

/**
	Receives messages from AP 
*/

void T_ReceiveMsg(struct AP_INFO *APInfoObj) {
    auto Context = new zmq::context_t(1);
    auto Subscriber = new zmq::socket_t(*Context, ZMQ_SUB);
    std::string IP_Port = APInfoObj->IP + ":" + std::to_string((std::stoi(APInfoObj->Port) + 1));
    Subscriber->connect("tcp://"+IP_Port);
    Subscriber->setsockopt(ZMQ_SUBSCRIBE, "AP_INFO", 7);
    zmq::message_t MessageReceived, Address;
    while (true) {

        Subscriber->recv(&Address);
        Subscriber->recv(&MessageReceived);

        std::string ReceivedMsg(static_cast<char *>(MessageReceived.data()), MessageReceived.size());
        CLUSTERINFO ClusterInfo;
        ClusterInfo.ParseFromString(ReceivedMsg);
        if(ClusterInfo.datatype() == "CARINFO") {
		/* Message with details of cars(position, speed, direction etc.) */
            VisualObj->StoreCarsInfo(&ClusterInfo);
        } else if (ClusterInfo.datatype() == "CLUSTERINFO") {
		/* Message with details of cluster status */
            VisualObj->CarVsMCDetails(&ClusterInfo);
        }
    }

}


/**
	Populates CarsList[] with all cars details when message with car details arrives 
*/

void TRAFFIC_VISUALS::StoreCarsInfo(CLUSTERINFO *CarsInfo) {
    std::string CarsInfoStr = CarsInfo->cmlist();
    std::string SrcID = CarsInfo->src_id();
    std::stringstream ss(CarsInfoStr);
    int i = 0, j = 0;
    int NoOfValues = 6;
    auto CarsList = ListOfCars[CarsInfo->src_id()];
    while( ss.good() ) {
        std::string substr;
        getline( ss, substr, ',' );
        if(!substr.empty()) {
            switch(i % NoOfValues) {
                case 0: CarsList[j].ID = substr;
                        break;
                case 1: CarsList[j].X = std::stod(substr);
                        break;
                case 2: CarsList[j].Y = std::stod(substr);
                        break;
                case 3: CarsList[j].Speed = std::stod(substr);
                        break;
                case 4: CarsList[j].Dir = std::stod(substr);
                        break;
                case 5: CarsList[j].State = substr;
                        CarsList[j].Manager = SrcID;
                        j++;
            }
        }
        i++;
    }
    NumOfCarsExisting[CarsInfo->src_id()] = j;
	/* upon change of cars details enable RedisplayEnable to draw cars again */
    RedisplayEnable = true;
}


/** 
	Draws single line
	@param X1 Line starting x coordinate
	@param Y1 Line starting y coordinate
	@param X2 Line ending x coordinate
	@param Y2 Line ending y coordinate
*/

void DrawLine(float X1, float Y1, float X2, float Y2) {
    glEnable(GL_LINE_SMOOTH);
    glBegin( GL_LINES );                //draw solid polygon
    glVertex2i( X1, Y1 );
    glVertex2i( X2, Y2 );
    glEnd();
}


/** 
	Draws ploygon 
		
	@param X1 Ploygon's lower x coordinate
	@param Y1 Ploygon's lower y coordinate
	@param X2 Ploygon's upper x coordinate
	@param Y2 Ploygon's upper y coordinate
*/

void DrawPolygon(float X1, float Y1, float X2, float Y2) {
    glRectf(X1,Y1,X2,Y2);
}


/**
	Draws car
	@param X1 Car's x coordinate
	@param Y1 Car's y coordinate
*/

void DrawCar(float X1, float Y1) {
    glEnable( GL_LINE_SMOOTH );
    glEnable( GL_POLYGON_SMOOTH );
    glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
    glHint( GL_POLYGON_SMOOTH_HINT, GL_NICEST );
    glBegin(GL_TRIANGLES);
    glVertex2f(X1-3,Y1);
    glVertex2f(X1+3,Y1);
    glVertex2f(X1,Y1+4);
    glEnd();
    glRectf(X1 - 3,Y1 - 5,X1 + 3,Y1);
}


/**
	Re-draw all the objects on the window if there is a change
*/

void Drive(int e) {
    if(VisualObj->RedisplayEnable) {
        glutPostRedisplay();
        VisualObj->RedisplayEnable = false;
    }
	/* Calls Drive function() every 10 milliseconds */
    glutTimerFunc(10, Drive, -1);
}


/**
	Draws roads of the scenario
*/

void TRAFFIC_VISUALS::DrawRoads() {
    double Start_X = 0, Start_Y = 0;
    Start_Y = 10.0 + Road_Width;

	/* Draws plogon cubes(buildins in between the roads) of manhattan scenario */
    glColor3f(0.0f, 0.3f, 0.0f);
    for (int i = 1; i <= No_Of_Y_Intersec - 1; i++) {
        Start_X += 10.0 + Road_Width;
        for (int j = 1; j <= No_Of_X_Intersec - 1; j++) {
            DrawPolygon(Start_X, Start_Y, Start_X + Road_Length - Road_Width, Start_Y + Road_Length - Road_Width);
            Start_X += Road_Length;
        }
        Start_Y += Road_Length;
        Start_X = 0;
    }

	/* Draw verticle roads */
    glColor3f(0.0f, 0.0f, 0.0f);
    Start_Y = 0.0;
    for (int j = 1; j <= No_Of_Y_Intersec * 2; j++) {
        Start_X = 0;
        if ((j % 2) != 0 && j == 1)
            Start_Y += 10.0;
        else if ((j % 2) == 0)
            Start_Y += Road_Width;
        else
            Start_Y += Road_Length - Road_Width;
        for (int i = 1; i <= No_Of_X_Intersec + 1; i++) {
            if(i == 1 || i == (No_Of_X_Intersec + 1)) {
                DrawLine(Start_X, Start_Y, Start_X + 10.0, Start_Y);
                Start_X += 10.0 + Road_Width;
            } else {
                DrawLine(Start_X, Start_Y, Start_X + Road_Length - Road_Width, Start_Y);
                Start_X += Road_Length;
            }
        }
    }
	
	/* Draw horizontal roads */
    glColor3f(0.0f, 0.0f, 0.0f);
    Start_X = 0.0;
    for (int j = 1; j <= No_Of_X_Intersec * 2; j++) {
        Start_Y = 0;
        if ((j % 2) != 0 && j == 1)
            Start_X += 10.0;
        else if ((j % 2) == 0)
            Start_X += Road_Width;
        else
            Start_X += Road_Length - Road_Width;
        for (int i = 1; i <= No_Of_Y_Intersec + 1; i++) {
            if(i == 1 || i == (No_Of_Y_Intersec + 1)) {
                DrawLine(Start_X, Start_Y, Start_X , Start_Y + 10.0);
                Start_Y += 10.0 + Road_Width;
            } else {
                DrawLine(Start_X, Start_Y, Start_X , Start_Y + Road_Length - Road_Width);
                Start_Y += Road_Length;
            }
        }
    }
}


/**
	Draws text 
	@param string Contains the string to be drawn
	@param x, y, and z The coordinates where text should be drawn
*/

void drawBitmapText(char *string,float x,float y,float z)
{
    char *c;
    glRasterPos3f(x, y,z);

    for (c=string; *c != '\0'; c++)
    {

        /*glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *c)*/
        if(*c == '?') {
            y -= 10;
            glRasterPos3f(x, y,z);
            continue;
        }
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *c);
    }
}

/* Coordinates of mouse click to draw car details */
float MarkCar_X;
float MarkCar_Y;

/* MarkCar enables whether car details should be drawn, initially set to false */
bool MarkCar = false;

/* ID of the car whose details are to be drawn */
std::string MarkedCarID;

void DrawCoOrdinates();

/* Enables to draw cluster details */
bool EnableMarkAP = false;

/* ID of the cluster to be drawn */
std::string MarkedAP;

/* Enables to check or not check if the clicked coordinates proximity has any car */
bool Nocheck = false;
bool MarkOnlyCar = false;

/* Coordinates where scenario details are to be drawn */
float Mark_ScenarioX;
float Mark_ScenarioY;

/* Enables to draw scenario */
bool EnableMarkScenario;


/**
	Draw cars details when clicked on a particular car 
*/

void DrawCarDetails(std::string ID, double X, double Y, double Speed, double Dir, std::string State) {
    if(MarkCar) {
        MarkCar_X += 20;
        MarkCar_Y += 20;

        std::string Pos = "ID=" + ID + "?" + "Pos=" + std::to_string((int)MarkCar_X) + "," + std::to_string((int)MarkCar_Y) + "?";
        Pos += "Speed=" + std::to_string((int)Speed) + "mps?" + "Dir=" + std::to_string((int)Dir) + "?State=" + State;
        int CountOfNewLines= 4;
        if(State == "CM" || State == "CH") {
            Pos += "?ClusterID=" + VisualObj->Car_vs_MC[ID];
            CountOfNewLines+=1;
        }
        glColor3f(0.7f, 0.0f, 0.0f);
        DrawPolygon(MarkCar_X - 15, MarkCar_Y - (CountOfNewLines * 15), MarkCar_X + 100, MarkCar_Y + 10);
        glColor3f(1.0f, 1.0f, 1.0f);
        drawBitmapText((char*)Pos.c_str(), MarkCar_X - 12, MarkCar_Y-2, 0);
    }
}


/**
	Draws all the cars details sent by AP
	in different colors depending upon their state
*/

void TRAFFIC_VISUALS::DrawCars() {
    for (auto it = ListOfCars.begin(); it != ListOfCars.end(); it++) {
        struct MyCar *CarsList = it->second;
    	for (int i = 0; i < NumOfCarsExisting[it->first]; i++) {
    	    if(MarkCar && MarkOnlyCar && (CarsList[i].ID == MarkedCarID)) {
                glColor3f(0.0f, 0.0f, 0.5f);
    	    } else {
                if (CarsList[i].State == "CM")
                    glColor3f(1.0f, 1.0f, 0.0f);
                else if (CarsList[i].State == "CH")
                    glColor3f(1.0f, 0.0f, 0.0f);
                else
                    glColor3f(1.0f, 1.0f, 1.0f);
            }
        	glPushMatrix();
        	glTranslatef(CarsList[i].X, CarsList[i].Y, 0.0f);
        	glRotatef(360.0 - CarsList[i].Dir, 0.0f, 0.0f, 1.0f);
        	glTranslatef(-CarsList[i].X, -CarsList[i].Y, 0.0f);
        	DrawCar(CarsList[i].X, CarsList[i].Y);
        	glPopMatrix();
            auto _X = (int)CarsList[i].X;
            auto _Y = (int)CarsList[i].Y;

		/* Nocheck - Once clicked on car, details are shown on a red banner
		No need to check if clicked coordinates match with the coordinates of any car on every repaint*/
            if(MarkCar && !Nocheck) {
                if (check(_X + 4, _Y + 4, _X + 4, _Y - 4, _X - 4, _Y - 4, _X - 4, _Y + 4, MarkCar_X, MarkCar_Y)) {
                    MarkedCarID = CarsList[i].ID;
                    MarkedAP = it->first;
                    Nocheck = true;
                    DrawCarDetails(CarsList[i].ID, CarsList[i].X, CarsList[i].Y, CarsList[i].Speed, CarsList[i].Dir, CarsList[i].State);
                    MarkOnlyCar = true;
                }
            } else if(MarkCar && Nocheck) {
                if(CarsList[i].ID == MarkedCarID) {
                    MarkCar_X = CarsList[i].X ;
                    MarkCar_Y = CarsList[i].Y;
                    DrawCarDetails(CarsList[i].ID, CarsList[i].X, CarsList[i].Y, CarsList[i].Speed, CarsList[i].Dir, CarsList[i].State);
                }
            }
        }
    }
}


/**
	Draws circle
	@param cx X cordinate of the centre of the circle
	@param cy Y cordinate of the centre of the circle
	@param numsegments number of arcs used to draw circle
*/

void DrawCircle(float cx, float cy, float r, int num_segments, std::string MC_ID)
{
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_POINTS);
    glVertex2f(cx, cy);
    glEnd();
    float theta = 2 * 3.1415926 / float(num_segments);
    float c = cosf(theta);//precalculate the sine and cosine
    float s = sinf(theta);
    float t;

    float x = r;//we start at angle = 0
    float y = 0;

    glBegin(GL_LINE_LOOP);
    for(int ii = 0; ii < num_segments; ii++)
    {
        glVertex2f(x + cx, y + cy);//output vertex

        //apply the rotation matrix
        t = x;
        x = c * x - s * y;
        y = s * t + c * y;
    }
    glEnd();
}

/* coordinates of mouse click to draw cluster details */
double MarkAP_X;
double MarkAP_Y;


/**
	Draws(displays) micro cloud details when clicked at the centre of the micro cloud
*/
void TRAFFIC_VISUALS::DrawMC() {
    for (auto it = MC_Details.begin(); it != MC_Details.end(); it++) {
        auto Obj = it->second;
        DrawCircle(Obj->PosX, Obj->PosY, Obj->Radius, 100, it->first);
        if(EnableMarkAP) {
            int CountOfnewLines = 0;
            double Distance = sqrt(pow(((int)Obj->PosX - (int)MarkAP_X), 2) + pow(((int)Obj->PosY - (int)MarkAP_Y), 2));
		/* If clicked distance is less than 5 metres draw cluster details */
            if (Distance <= 5.0) {
                std::string MCInfo = "ID=" + it->first + "?" + "Pos=(" + std::to_string((int) Obj->PosX) + "," +
                                     std::to_string((int) Obj->PosY) + ")?" + "Manager=" + Obj->Manager ;
                std::string CMInfo = "?Members=";
                std::string CHInfo = "?CH=";
                CountOfnewLines += 3;

                for (auto jt = ListOfCars.begin(); jt != ListOfCars.end(); jt++) {
                    struct MyCar *CarsList = jt->second;
                    int count = 1;
                    for (int i = 0; i < NumOfCarsExisting[jt->first]; i++) {
                        if(count % 3 == 0) {
                            CountOfnewLines += 1;
                            CMInfo+="?";
                            count = 1;
                        }
                        if ((Car_vs_MC[CarsList[i].ID] == it->first)) {
                            if(CarsList[i].State == "CM") {
                                CMInfo += CarsList[i].ID + ",";
                                count++;
                            } else if(CarsList[i].State == "CH") {
                                CountOfnewLines += 1;
                                CHInfo += CarsList[i].ID;
                                count++;
                            }
                        }
                    }
                }
                MCInfo += CHInfo + CMInfo;
                glColor3f(0.7f, 0.0f, 0.0f);
                DrawPolygon(MarkAP_X - 15, MarkAP_Y - (CountOfnewLines * 15), MarkAP_X + 110, MarkAP_Y + 10);
                glColor3f(1.0f, 1.0f, 1.0f);
                drawBitmapText((char*)MCInfo.c_str(), MarkAP_X - 12, MarkAP_Y, 0);
            }
        }
    }
}


/**
	Draws(displays) coordinates of the mouse click
*/

void DrawCoOrdinates() {
    if(MarkCar && !MarkOnlyCar) {
        glColor3f(1.0f, 1.0f, 1.0f);
        DrawPolygon(MarkCar_X - 15, MarkCar_Y - 10, MarkCar_X + 60, MarkCar_Y + 15);
        std::string Pos = std::to_string((int)MarkCar_X) + "," + std::to_string((int)MarkCar_Y);
        glColor3f(0.0f, 0.0f, 0.0f);
        drawBitmapText((char*)Pos.c_str(), MarkCar_X - 12, MarkCar_Y, 0);
    }
}


/**
	Draw(display) scenario details on a mouse centre click anywhere on the window
*/

void TRAFFIC_VISUALS::DrawScenario() {
    if(EnableMarkScenario) {
        glColor3f(0.7f, 0.0f, 0.0f);
        int CountOfnewLines = 6;
        DrawPolygon(Mark_ScenarioX - 15, Mark_ScenarioY - (CountOfnewLines * 10), Mark_ScenarioX + 150, Mark_ScenarioY + 10);
        std::string ScenarioDetails = "Scenario = (" + std::to_string(W_Size_X) + "," + std::to_string(W_Size_Y)+ ")?Road Length = " + std::to_string(Road_Length) +"m?Road Width = " + std::to_string(Road_Width) +
                "m?Intersections X = " + std::to_string(No_Of_X_Intersec) + "?Intersection Y = " + std::to_string(No_Of_Y_Intersec) +"?TotalCars = " + std::to_string(TotalCars);
        glColor3f(1.0f, 1.0f, 1.0f);
        drawBitmapText((char*)ScenarioDetails.c_str(), Mark_ScenarioX - 12, Mark_ScenarioY, 0);
    }
}


/** 
	Handler for window-repaint event. Call back when the window first appears and
   	whenever the window needs to be re-painted. 
*/
void display() {
    glClear(GL_COLOR_BUFFER_BIT);    // Clear the color buffer
    glMatrixMode(GL_MODELVIEW);      // To operate on Model-View matrix
    glLoadIdentity();                // Reset the model-view matrix


    VisualObj->DrawRoads();
    VisualObj->DrawCars();
    VisualObj->DrawMC();
    DrawCoOrdinates();
    VisualObj->DrawScenario();
    glFlush();   // Render now
}


double W_New_Width, W_New_Height;

/** 
	Called automatically when the window size is changed 
	This adjusts the aspect ratio
*/
void reshape(GLsizei width, GLsizei height) {  // GLsizei for non-negative integer
    if (height == 0) height = 1;                // To prevent divide by 0
    GLfloat aspect = (GLfloat)width / (GLfloat)height;

    // Set the viewport to cover the new window
    glViewport(0, 0, width, height);

    // Set the aspect ratio of the clipping area to match the viewport
    glMatrixMode(GL_PROJECTION);  // To operate on the Projection matrix
    glLoadIdentity();             // Reset the projection matrix
    if (width >= height) {
        // aspect >= 1, set the height from -1 to 1, with larger width
        gluOrtho2D(0.0 * aspect, W_Size_X * aspect, 0.0, W_Size_Y);
    } else {
        // aspect < 1, set the width to -1 to 1, with larger height
        gluOrtho2D(0.0, W_Size_X, 0.0 / aspect, W_Size_Y / aspect);
    }
}


/**
	Create receiver thread each each AP
*/

void TRAFFIC_VISUALS::SpawnAPReceivers() {
    std::vector<std::thread *> z;
    for (auto it = AP_Details.begin(); it != AP_Details.end(); it++) {
        z.push_back(new std::thread(T_ReceiveMsg, it->second));
    }
}


/**
	Records mouse clicks and call appropriate callback function
*/

void mouse(int button, int state, int x, int y) {
    y = glutGet(GLUT_WINDOW_HEIGHT) - y;

    x = (float)x * ((float)W_Size_X / (float)glutGet(GLUT_WINDOW_WIDTH));
    y = (float)y * ((float)W_Size_Y / (float)glutGet(GLUT_WINDOW_HEIGHT));

    std::cout << "Position Clicked (X,Y) = " << x << " , " << y << std::endl;

    if(GLUT_LEFT_BUTTON == button && GLUT_DOWN == state && !MarkCar && !EnableMarkAP) { // Enables to draw car details
        MarkCar_X = x;
        MarkCar_Y = y;
        MarkCar = true;
    } else if ((GLUT_LEFT_BUTTON == button && GLUT_DOWN == state) && MarkCar) { // Enables to remove any previous draws
        MarkCar = false;
        Nocheck = false;
        MarkOnlyCar = false;
        EnableMarkAP = false;
        EnableMarkScenario = false;
    } else if ((GLUT_RIGHT_BUTTON == button && GLUT_DOWN == state)) { // Enables to draw cluster details
        MarkAP_X = x;
        MarkAP_Y = y;
        EnableMarkAP = true;
    } else if ((GLUT_LEFT_BUTTON == button && GLUT_DOWN == state)) { // Enables to remove previously drawn cluster details
        EnableMarkAP = false;
    } else if ((GLUT_MIDDLE_BUTTON == button && GLUT_DOWN == state)) { // Enables to draw scenario details
        Mark_ScenarioX = x;
        Mark_ScenarioY = y;
        EnableMarkScenario = true;
    }
	/* Enable re-drawing on any change in mouse events */
    VisualObj->RedisplayEnable = true;
}


/** 
	Main function: GLUT runs as a console application starting at main() 
*/
int main(int argc, char** argv) {
    VisualObj = new TRAFFIC_VISUALS();
    W_Size_X = VisualObj->Road_Length * (VisualObj->No_Of_X_Intersec - 1) + 40.0;
    W_Size_Y = VisualObj->Road_Length * (VisualObj->No_Of_Y_Intersec - 1) + 40.0;
    W_New_Width = W_Size_X;
    W_New_Height = W_Size_Y;

    glutInit(&argc, argv);          // Initialize GLUT
    glutInitWindowSize(W_Size_X, W_Size_Y);   // Set the window's initial width & height - non-square
    glutInitWindowPosition(0, 0); // Position the window's initial top-left corner
    glutCreateWindow("Model Transform");  // Create window with the given title
    glutMouseFunc(mouse);
    glutDisplayFunc(display);       // Register callback handler for window re-paint event
    glutReshapeFunc(reshape);       // Register callback handler for window re-size event
    initGL();                       // Our own OpenGL initialization
    glutTimerFunc(10, Drive, -1);

    VisualObj->SpawnAPReceivers();
    glutMainLoop();              // Enter the infinite event-processing loop
    return 0;
}
