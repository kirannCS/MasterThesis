//
// Created by kiran.narayanaswamy on 12/10/18.
//


#include "Visuals.h"
#include "../../../lib/pugixml/pugixml.hpp"
#include "../../../lib/pugixml/pugixml.cpp"


/** 
        Constructor that reads XML Config file to populate required variables 
*/

TRAFFIC_VISUALS::TRAFFIC_VISUALS() {
    pugi::xml_document doc;
    pugi::xml_node tools = doc.child("VMCPI").child("config");
    if (!doc.load_file("../config/visuals/config.xml")) {
        std::cout << "ERROR:: Config File Missing ../config/visuals/config.xml !!\n";
    } else {
        tools = doc.child("VMCPI").child("config");

        for (pugi::xml_node_iterator it = tools.begin(); it != tools.end(); ++it) {
            for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait) {
                std::string name = ait->name();
                std::cout << "Name = " << name << " Vaue = " << ait->value() << std::endl;
                if (name == "StreetMargin")
                    Road_Width = std::stol(ait->value());
                else if (name == "StreetLength")
                    Road_Length = std::stol(ait->value());
                else if (name == "Lanes")
                    No_Of_Lanes = std::stol(ait->value());
                else if (name == "X")
                    No_Of_X_Intersec = std::stol(ait->value());
                else if (name == "Y")
                    No_Of_Y_Intersec = std::stol(ait->value());
                else if (name == "TotalCars") {
                    TotalCars = std::stol(ait->value());
                }
            }
        }
    }

    if (!doc.load_file("../config/ap/config.xml")) {
        std::cout << "ERROR:: Config File Missing ../config/ap/config.xml !!\n";
    } else {
        tools = doc.child("VMCPI").child("config");
        bool Parse = true;
        std::string Fname, Sname, Value;
        std::string AttrName, Attrvalue, Manager;
        for (pugi::xml_node_iterator it = tools.begin(); it != tools.end(); ++it) {
            Fname = it->name();
            if (Fname == "BS" || Fname == "RSU") {
                auto Obj = new struct AP_INFO;
                for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait) {
                    Sname = ait->name();
                    Value = ait->value();
                    if (Sname == "id")
                        Obj->ID = Value;
                    else if (Sname == "ip")
                        Obj->IP = Value;
                    else if (Sname == "port")
                        Obj->Port = Value;
                    else if (Sname == "x")
                        Obj->PosX = std::stod(Value);
                    else if (Sname == "y")
                        Obj->PosY = std::stod(Value);
                }
                AP_Details[Obj->ID] = Obj;
		        ListOfCars[Obj->ID] = new struct MyCar[TotalCars + 5];
            } else if (Fname == "MICROCLOUD") {
                auto Obj = new struct MC_INFO;
                for (pugi::xml_attribute_iterator ait = it->attributes_begin(); ait != it->attributes_end(); ++ait) {
                    Sname = ait->name();
                    Value = ait->value();
                    if (Sname == "id")
                        Obj->ID = Value;
                    else if (Sname == "x")
                        Obj->PosX = std::stod(Value);
                    else if (Sname == "y")
                        Obj->PosY = std::stod(Value);
                    else if (Sname == "manager")
                        Obj->Manager = Value;
                    else if (Sname == "mc_radius")
                        Obj->Radius = std::stod(Value);
                }
                MC_Details[Obj->ID] = Obj;
            }
        }
        for(auto it = AP_Details.begin(); it != AP_Details.end(); it++) {
            auto Obj = it->second;
            std::cout << " " << Obj->ID << " " << Obj->PosX << " " << Obj->PosY << " " << Obj->IP << " " << Obj->Port  << "\n";
        }
        for(auto it = MC_Details.begin(); it != MC_Details.end(); it++) {
            auto Obj = it->second;
            std::cout << "it->first " << it->first << " " << Obj->ID << " " << Obj->PosX << " " << Obj->PosY << " " << Obj->Manager << " " << Obj->Radius  << "\n";
        }
    }
    RedisplayEnable = false;
}


/** 
	Calculates and returns the area of a triangle 
*/

float area(int x1, int y1, int x2, int y2,
           int x3, int y3)
{
    return abs((x1 * (y2 - y3) + x2 * (y3 - y1) +
                x3 * (y1 - y2)) / 2.0);
}

/* A function to check whether point P(x, y)
   lies inside the rectangle formed by A(x1, y1),
   B(x2, y2), C(x3, y3) and D(x4, y4) */
bool check(int x1, int y1, int x2, int y2, int x3,
           int y3, int x4, int y4, int x, int y)
{
    /* Calculate area of rectangle ABCD */
    float A = area(x1, y1, x2, y2, x3, y3) +
              area(x1, y1, x4, y4, x3, y3);

    /* Calculate area of triangle PAB */
    float A1 = area(x, y, x1, y1, x2, y2);

    /* Calculate area of triangle PBC */
    float A2 = area(x, y, x2, y2, x3, y3);

    /* Calculate area of triangle PCD */
    float A3 = area(x, y, x3, y3, x4, y4);

    /* Calculate area of triangle PAD */
    float A4 = area(x, y, x1, y1, x4, y4);

    /* Check if sum of A1, A2, A3 and A4
       is same as A */
    return (A == A1 + A2 + A3 + A4);
}









