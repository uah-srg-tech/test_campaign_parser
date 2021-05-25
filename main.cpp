/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: user
 *
 * Created on 30 de julio de 2019, 18:04
  */

#include <iostream>
#include "test_campaign_parser.h"

using namespace std;

int main(int argc, char** argv)
{
    int32_t status = 0;
    uint32_t proceduresParsed = 0;
    string envScenario;

    test_campaign_parser parser(argv[1]);
    
    if((status = parser.createCampaignFromFullCampaign(proceduresParsed,
            envScenario)) != 0)
    {
        cout << endl << parser.displayCreateCampaignFromFullCampaignError() << endl;
        getchar();
        return 1;
    }
    cout << endl << "Parsed " << proceduresParsed << " procedures at " << envScenario << endl;
    getchar();
    return 0;
}

