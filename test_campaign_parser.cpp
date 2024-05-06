/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   test_campaign_parser.cpp
 * Author: user
 * 
 * Created on 30 de julio de 2019, 18:05
 */

#include "test_campaign_parser.h"
#include "../massiva/XMLParsingTools/XMLTools.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <sys/stat.h>                   /* mkdir */

using namespace std;

test_campaign_parser::test_campaign_parser(const char * config_filename_c_str)
{    
    char currentDir[FILENAME_MAX];
    /* get current dir for full minimal dir */
#ifdef __CYGWIN__
    getwd(currentDir);
#else
    getcwd(currentDir, FILENAME_MAX);
#endif
    
    if(config_filename_c_str == NULL)
    {
        config_filename = (string)currentDir + "\\test_campaign_parser.ini";
    }
    else
    {
        ostringstream config_filename_ss;
        config_filename_ss << config_filename_c_str;
        config_filename = config_filename_ss.str();
    }
    scenarioRef = -1;
    status = NO_ERROR;
}

uint32_t test_campaign_parser::createCampaignFromFullCampaign(uint32_t &proceduresParsedRef,
        string &envScenario)
{
    ifstream configFile;
    
    configFile.open(config_filename.c_str());
    if(!configFile.is_open())
    {
        status = CANT_OPEN_CONFIG_FILE;
        return 1;
    }
    if(!getline(configFile, test_campaign_filename))
    {
        configFile.close();
        errorInfo = config_filename;
        status = FILE_NOT_VALID;
        return 1;
    }
    if(!getline(configFile, environment))
    {
        configFile.close();
        errorInfo = config_filename;
        status = FILE_NOT_VALID;
        return 1;
    }
    if(!getline(configFile, scenario))
    {
        configFile.close();
        errorInfo = config_filename;
        status = FILE_NOT_VALID;
        return 1;
    }
    configFile.close();
    envScenario = "\"" + environment + "\" Scenario \"" + scenario + "\"";
    
    /* check if scenario exists in environment */
    size_t pos = test_campaign_filename.find_last_of("\\");
    if(pos == string::npos)
        pos = test_campaign_filename.find_last_of("/");
    output_filename = test_campaign_filename.substr(0, pos) + "/" + environment;

    LIBXML_TEST_VERSION
    xmlDocPtr doc = xmlParseFile(output_filename.c_str());
    if(doc == NULL)
    {
        errorInfo = output_filename;
        status = FILE_NOT_FOUND;
        return 1;
    }
    checkScenarioInEnvironment(doc);
    xmlFreeDoc(doc);
    xmlCleanupParser();
    if(status != NO_ERROR)
        return 1;
    
    output_filename = test_campaign_filename.substr(0, (test_campaign_filename.length()-4)) + "_campaign.xmi";
    campaign_file.open(output_filename.c_str(), ios::out|ios::trunc);
    if (campaign_file.fail())
    {
        status = CANT_CREATE_CAMPAIGN_FILE;
        return 1;
    }
    uint32_t parseStatus = parseTestCampaignFile(proceduresParsedRef);
    campaign_file.close();
    return parseStatus;
}

void test_campaign_parser::checkScenarioInEnvironment(xmlDocPtr doc)
{
    xmlNodePtr root = NULL;
    uint32_t numberOfScenarios = 0;
    
    xmlParsingStatus = 0;
    root = xmlDocGetRootElement(doc);
    if(root == NULL)
    {
        errorInfo = environment;
        status = FILE_PARSING_ERROR;
        return;
    }
    GetXMLNumChildren(root, &numberOfScenarios);
    uint32_t currentScenario = 0;
    for(currentScenario=0; currentScenario<numberOfScenarios; ++currentScenario)
    {
        xmlNodePtr scenarioHandle = NULL;
        char scenarioAuxString[50];
        if((xmlParsingStatus = GetXMLChildElementByIndex(root, currentScenario,
                &scenarioHandle)) != 0)
        {
            status = XML_PARSING_ERROR;
            break;
        }
        if((xmlParsingStatus = GetXMLAttributeValueByName(scenarioHandle,
                "name", scenarioAuxString, 50)) != 0)
        {
            status = XML_PARSING_ERROR;
            break;
        }
        if(scenario.compare(scenarioAuxString) == 0)
        {
            scenarioRef = (int32_t)currentScenario;
            break;
        }
    }
    if(currentScenario == numberOfScenarios)
    {
        errorInfo = environment;
        status = SCENARIO_NOT_FOUND;
    }
    return;
}

string test_campaign_parser::displayCreateCampaignFromFullCampaignError()
{
    ostringstream error_ss;
    char auxMsg[70];
    
    error_ss << "At line " << current_line << ", ";
    switch(status)
    {
        case CANT_OPEN_CONFIG_FILE:
            error_ss << "Can't open config file \"" << config_filename << "\"";
            break;
            
        case CANT_CREATE_CAMPAIGN_FILE:
            error_ss << "Can't create campaign file \"" << output_filename << "\"";
            break;
            
        case FILE_NOT_FOUND:
            error_ss << "Can't open file \"" << errorInfo << "\"" << endl;
            break;
            
        case FILE_NOT_VALID:
            error_ss << "File \"" << errorInfo << "\" is not valid" << endl;
            break;
            
        case FILE_PARSING_ERROR:
            error_ss << "Parsing error at file \"" << errorInfo << "\"" << endl;
            break;
            
        case XML_PARSING_ERROR:
            XMLerrorInfo(xmlParsingStatus, auxMsg, 70);
            error_ss << "XML parsing error: " << auxMsg;
            break;
            
        case SCENARIO_NOT_FOUND:
            error_ss << "Scenario \"" << scenario << "\" not found at \"" << errorInfo << "\"" << endl;
            break;
            
        case CANT_FIND_NAME:
            error_ss << "Can't find \"name\" tag" << endl;
            break;
            
        case CANT_FIND_END_QUOTES:
            error_ss << "Can't find end quotes \" at " << errorInfo << endl;
            break;
                        
        case CANT_CHANGE_DIRECTORY:
            error_ss << "Can't change directoy to " << output_filename << endl;
            break;
            
        case CANT_CREATE_DIR:
            error_ss << "Can't create \"" << errorInfo << "\" folder";
            break;
            
        case CANT_CREATE_TEST_PROC_FILE:
            error_ss << "Can't create test proc file \"" << output_filename << "\"";
            break;
            
        case TEST_PROC_FILE_NOT_CLOSED:
            error_ss << "Can't create new test proc file as previous one \""
                    << output_filename << "\" was not closed";
            break;
            
        case TEST_PROC_FILE_NOT_OPENED:
            error_ss << "Can't close test proc file as no one has been opened";
            break;
            
        case EXPORT_FILTER_FILE_NOT_CLOSED:
            error_ss << "Can't create new export/filter file as previous one \""
                    << output_filename << "\" was not closed";
            break;
            
        case EXPORT_FILTER_FILE_NOT_OPENED:
            error_ss << "Can't close export/filter file file as no one has been opened";
            break;
            
        case CANT_FIND_EXPORT_FILTER_TYPE:
            error_ss << "Can't find export/filter type" << endl;
            break;
            
        case CANT_CREATE_EXPORT_FILTER_FILE:
            error_ss << "Can't create export/filter file \"" << output_filename << "\"";
            break;
            
        case EXPORT_FILTER_WRONG_TAG:
            error_ss << "Wrong export/filter file \"" << errorInfo << "\"";
            break;
            
        case REFERENCE_TOO_BIG:
            error_ss << "Reference values " << errorInfo << "\" too big";
            break;
            
        default:
            error_ss << "Unknown error " << +status;
            break;
    }
    return error_ss.str();
}

uint32_t test_campaign_parser::parseTestCampaignFile(uint32_t & proceduresParsedRef)
{
    ifstream test_campaign_file;
    string test_campaign_line;
    bool skipLine = false;
    string gssHeaderTag;
    uint32_t number_of_lines = 0;
    size_t pos = 0, posEnd = 0;
    vector<string> references;
    bool lastLineWasInputTag = false;
    bool parsingTMTC = false;
    vector<string> test_cases_name;
    vector<string> test_cases_prevAction;
    vector<string> test_cases_prevActionParam;
            
    //open test_campaign file
    test_campaign_file.open(test_campaign_filename.c_str());
    if (!test_campaign_file.is_open())
    {
        errorInfo = test_campaign_filename;
        status = FILE_NOT_FOUND;
        return 1;
    }
    
    //pass 0: get number of lines
    while (getline(test_campaign_file, test_campaign_line))
        ++number_of_lines;
    test_campaign_file.clear();
    test_campaign_file.seekg(0, ios::beg);
    
    //pass 2: get numbering of exports / filters
    references.push_back("");
    while(getline(test_campaign_file, test_campaign_line))
    {
        if((pos = test_campaign_line.find("<gss_")) != string::npos)
        {
            if((pos = test_campaign_line.substr(0).find("\"")) == string::npos)
            {
                status = CANT_FIND_EXPORT_FILTER_TYPE;
                return 1;
            }
            if((posEnd = test_campaign_line.substr(pos+1).find("\"")) == string::npos)
            {
                errorInfo = "export/filter name";
                status = CANT_FIND_END_QUOTES;
                return 1;
            }
            references.push_back(test_campaign_line.substr(pos+1, posEnd));
        }
    }
    test_campaign_file.clear();
    test_campaign_file.seekg(0, ios::beg);
    
    //pass 3a: header - get name and check scenario
    current_line = 3;
    //get 3 first lines to get name
    getline(test_campaign_file, test_campaign_line); //<?xml version
    getline(test_campaign_file, test_campaign_line); //<xmi:XMI
    getline(test_campaign_file, test_campaign_line); //<gss:GSSCampaignCampaign
    if(test_campaign_line.substr(2, 24).compare("<gss:GSSCampaignCampaign") != 0)
    {
        test_campaign_file.close();
        errorInfo = test_campaign_filename;
        status = FILE_NOT_VALID;
        return 1;
    }
    if((pos = test_campaign_line.find("name")) == string::npos)
    {
        status = CANT_FIND_NAME;
        return 1;
    }
    if((posEnd = test_campaign_line.substr(pos+6).find("\"")) == string::npos)
    {
        errorInfo = "<gss:GSSCampaignCampaign name";
        status = CANT_FIND_END_QUOTES;
        return 1;
    }
    test_campaign_name = test_campaign_line.substr(pos+6, posEnd);

    output_filename = test_campaign_filename.substr(0, test_campaign_filename.find_last_of("\\"));
    if(chdir(output_filename.c_str()) != 0)
    {
        status = CANT_CHANGE_DIRECTORY;
        return 1;
    }
    
    int32_t dirStatus = 0;
    /* create testCampaign directory if doesn't exist */
#if defined (_WIN32) || defined (__CYGWIN__)
    dirStatus = mkdir(test_campaign_name.c_str());
#else
    dirStatus = mkdir(test_campaign_name.c_str(), 0777);
#endif
    if((dirStatus == -1) && (errno != EEXIST))
    {
        errorInfo = test_campaign_name;
        status = CANT_CREATE_DIR;
        return 1;
    }
    
    if(chdir(test_campaign_name.c_str()) != 0)
    {
        output_filename = test_campaign_name;
        status = CANT_CHANGE_DIRECTORY;
        return 1;
    }
#if defined (_WIN32) || defined (__CYGWIN__)
    dirStatus = mkdir("TCs");
#else
    dirStatus = mkdir("TCs", 0777);
#endif
    if((dirStatus == -1) && (errno != EEXIST))
    {
        errorInfo = "TCs";
        status = CANT_CREATE_DIR;
        return 1;
    }
#if defined (_WIN32) || defined (__CYGWIN__)
    dirStatus = mkdir("TMs");
#else
    dirStatus = mkdir("TMs", 0777);
#endif
    if((dirStatus == -1) && (errno != EEXIST))
    {
        errorInfo = "TMs";
        status = CANT_CREATE_DIR;
        return 1;
    }
    
    //write Scenario Info
    campaign_file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
    campaign_file <<
            "<gss:GSSCampaignCampaign name=\"" << test_campaign_name << "\" "
            "xmlns:gss=\"http://srg.aut.uah.es/gss/campaign\">" << endl;
    campaign_file << "\t<Scenario>" << endl;
    campaign_file << "\t\t<environment href=\"" << environment << "#/\"/>" << endl;
    campaign_file << "\t\t<scenarioRef href=\"" << environment << "#//@scenario." << scenarioRef << "\"/>" << endl;
    //FIXME: Keep it as an input for the transformation
    campaign_file << "\t</Scenario>" << endl;
    campaign_file << "\t<Tests>" << endl;
    
    //pass 3b: parse line by line, perform replaces inline before writing
    while(getline(test_campaign_file, test_campaign_line))
    {
        current_line++;
        cout << "\r" << fixed << setprecision(2) << (current_line*100.0)/number_of_lines
                << "% (" << current_line << "/" << number_of_lines << ")";
        
        //0) add </input> if necessary
        if(((pos = test_campaign_line.find("</inputs>")) != string::npos) &&
                (lastLineWasInputTag == true))
        {
            test_proc_file << "        </input>" << endl;
        }
        lastLineWasInputTag = false;
        
        //1) remove &quot;
        while((pos = test_campaign_line.find("&quot;")) != string::npos)
            test_campaign_line.erase(pos, 6);
        
        //2) close file when necessary
        if((pos = test_campaign_line.find("</gss_1:GSSTestProcTestProc>")) != string::npos)
        {
            if(!skipLine)
            {
                if(!test_proc_file.is_open())
                {
                    status = TEST_PROC_FILE_NOT_OPENED;
                    return 1;
                }
                test_proc_file << "</gss:GSSTestProcTestProc>";
                test_proc_file.close();
            }
            skipLine = false;
            continue;
        }
        if(skipLine)
            continue;
        if(((pos = test_campaign_line.find("</gss_")) != string::npos) &&
                (test_campaign_line.substr(pos+6, 1).compare("1") != 0))
        {
            if(!test_proc_file.is_open())
            {
                status = EXPORT_FILTER_FILE_NOT_OPENED;
                return 1;
            }
            test_proc_file << "</gss:" << gssHeaderTag << ">";
            test_proc_file.close();
            parsingTMTC = false;
            continue;
        }
        
        //3.0) get test name at campaign and prevAction for writing it afterwards
        if((pos = test_campaign_line.find("<TestCase name")) != string::npos)
        {
            if((posEnd = test_campaign_line.substr(pos+16).find("\"")) == string::npos)
            {
                errorInfo = "TestCase name";
                status = CANT_FIND_END_QUOTES;
                return 1;
            }
            test_cases_name.push_back(test_campaign_line.substr(pos+16, posEnd));
            
            if((pos = test_campaign_line.find("prevAction")) != string::npos)
            {
                if((posEnd = test_campaign_line.substr(pos+12).find("\"")) == string::npos)
                {
                    errorInfo = "TestCase prevAction";
                    status = CANT_FIND_END_QUOTES;
                    return 1;
                }
                test_cases_prevAction.push_back(test_campaign_line.substr(pos+12, posEnd));
            }
            else
            {
                test_cases_prevAction.push_back("");
            }
            if((pos = test_campaign_line.find("prevActionParam")) != string::npos)
            {
                if((posEnd = test_campaign_line.substr(pos+17).find("\"")) == string::npos)
                {
                    errorInfo = "TestCase prevActionParam";
                    status = CANT_FIND_END_QUOTES;
                    return 1;
                }
                test_cases_prevActionParam.push_back(test_campaign_line.substr(pos+17, posEnd));
            }
            else
            {
                test_cases_prevActionParam.push_back("");
            }
        }
        
        //3) create new file when necessary
        //3.1) create new test proc
        if((pos = test_campaign_line.find("<gss_1:GSSTestProcTestProc name")) != string::npos)
        {
            if(test_proc_file.is_open())
            {
                test_proc_file.close();
                status = TEST_PROC_FILE_NOT_CLOSED;
                return 1;
            }
            
            //3.1.1) check scenarioRef
            size_t posScenarioId = 0;
            if((posScenarioId = test_campaign_line.find("scenario")) != string::npos)
            {
                if((posEnd = test_campaign_line.substr(posScenarioId+10).find("\"")) == string::npos)
                {
                    errorInfo = "scenario";
                    status = CANT_FIND_END_QUOTES;
                    return 1;
                }
            }
            if(test_campaign_line.substr(posScenarioId+10, posEnd) == scenario)
            {
                if((posEnd = test_campaign_line.substr(pos+33).find("\"")) == string::npos)
                {
                    errorInfo = "GSSTestProcTestProc name";
                    status = CANT_FIND_END_QUOTES;
                    return 1;
                }
                output_filename = test_campaign_line.substr(pos+33, posEnd) + ".xmi";
                test_proc_file.open(output_filename.c_str(), ios::out|ios::trunc);
                if (test_proc_file.fail())
                {
                    status = CANT_CREATE_TEST_PROC_FILE;
                    return 1;
                }

                test_proc_file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
                test_proc_file << "<gss:GSSTestProcTestProc name=\"" << test_campaign_line.substr(pos+33, posEnd) << "\" "
                        "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" " <<
                        "xmlns:gss=\"http://srg.aut.uah.es/gss/test_proc\" " <<
                        "xmlns:gss_1=\"http://srg.aut.uah.es/gss/filter\">" << endl;
                proceduresParsedRef++;
               
                campaign_file << "\t\t<TestCase name=\"" << test_cases_name.front() << "\"";
                test_cases_name.erase(test_cases_name.begin());
                
                if(test_cases_prevAction.front().compare("") != 0)
                {
                    campaign_file << " prevAction=\"" << test_cases_prevAction.front() << "\"" <<
                             " prevActionParam=\"" << test_cases_prevActionParam.front() << "\"";
                }
                //only write if not "", erase always
                test_cases_prevAction.erase(test_cases_prevAction.begin());
                test_cases_prevActionParam.erase(test_cases_prevActionParam.begin());
                campaign_file << ">" << endl;
                campaign_file << "\t\t\t<procedure href=\"" << test_campaign_name << "/" << output_filename << "#/\"/>" << endl;
                campaign_file << "\t\t</TestCase>" << endl;
            }
            else
            {
                skipLine = true;
            }
            continue;
        }
        //3.2) create new export/filter: gss_X; X > 1
        if(((pos = test_campaign_line.find("<gss_")) != string::npos) &&
                (test_campaign_line.substr(pos+5, 1).compare("1") != 0))
        {
            parsingTMTC = true;
            if(test_proc_file.is_open())
            {
                test_proc_file.close();
                status = EXPORT_FILTER_FILE_NOT_CLOSED;
                return 1;
            }
            //get type of export/filter
            size_t posType = 0;
            if((posType = test_campaign_line.substr(pos+7).find(" name")) == string::npos)
            {
                status = CANT_FIND_EXPORT_FILTER_TYPE;
                return 1;
            }
            gssHeaderTag = test_campaign_line.substr(pos+7, posType);
            if((gssHeaderTag != "GSSFilterMintermFilter") &&
                    (gssHeaderTag != "GSSFilterMaxtermFilter") &&
                    (gssHeaderTag != "GSSExportExport"))
                
            {
                errorInfo = gssHeaderTag;
                status = EXPORT_FILTER_WRONG_TAG;
                return 1;
            }
            
            if((posEnd = test_campaign_line.substr(pos+posType+14).find("\"")) == string::npos)
            {
                errorInfo = "test_proc name";
                status = CANT_FIND_END_QUOTES;
                return 1;
            }
            string gssHeaderAttrName = test_campaign_line.substr(pos+posType+14, posEnd);
            if(gssHeaderTag == "GSSExportExport")
                output_filename = "TCs/" + gssHeaderAttrName + ".xmi";
            else
                output_filename = "TMs/" + gssHeaderAttrName + ".xmi";
            test_proc_file.open(output_filename.c_str(), ios::out|ios::trunc);
            if (test_proc_file.fail())
            {
                status = CANT_CREATE_EXPORT_FILTER_FILE;
                return 1;
            }
            // replace in name " " for "_"
            size_t str_start = 0;
            while((str_start = gssHeaderAttrName.find(" ", str_start)) != std::string::npos) {
                gssHeaderAttrName.replace(str_start, 1, "_");
                str_start += 1;
            }
            test_proc_file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
            test_proc_file << "<gss:" << gssHeaderTag << " "
                    "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" ";
            if(gssHeaderTag == "GSSExportExport")
                test_proc_file << "xmlns:gss=\"http://srg.aut.uah.es/gss/export\" ";
            else
                test_proc_file << "xmlns:gss=\"http://srg.aut.uah.es/gss/filter\" ";
            test_proc_file << "xmlns:gss_1=\"http://srg.aut.uah.es/gss/format\" " <<
                    "name=\"" << gssHeaderAttrName << "\">" << endl;
            continue;
        }
        
        //4) change gss_XXXXXX#//@element for xmi#/
        if((pos = test_campaign_line.find(".gss_")) != string::npos)
            test_campaign_line.replace(pos, 22, ".xmi#/");

        //5) remove "@test_proc.XYYY/" from prev_step_idref and output_idref_from_prev_step
        if((pos = test_campaign_line.find("prev_step_idref")) != string::npos)
        {
            size_t posEnd = test_campaign_line.substr(pos+17).find("@");
            test_campaign_line.replace(pos+17, posEnd, "//");
        }
        if((pos = test_campaign_line.find("output_idref_from_prev_step")) != string::npos)
        {
            size_t posEnd = test_campaign_line.substr(pos+29).find("@");
            test_campaign_line.replace(pos+29, posEnd, "//");
        }
        //6) modify "gss_X:"
        if((((pos = test_campaign_line.find(":GSSFormat")) != string::npos)) ||
                ((!parsingTMTC) && ((pos = test_campaign_line.find(":GSSFilter")) != string::npos)))
        {
            test_campaign_line.replace(pos-1, 1, "1");//change gss_X to gss_1
        }
        else if((((pos = test_campaign_line.find(":GSSTestProc")) != string::npos)) ||
                ((parsingTMTC) && (((pos = test_campaign_line.find(":GSSFilter")) != string::npos)|| 
                ((pos = test_campaign_line.find(":GSSExport")) != string::npos))))
        {
            test_campaign_line.erase(pos-2, 2);//remove _X from gss_X
        }
        
        //update ccsds_solo references
        if(((pos = test_campaign_line.find("../ccsds_solo")) != string::npos) ||
                ((pos = test_campaign_line.find("../Ifaces")) != string::npos))
        {
            if(test_campaign_line.find("FieldRef") != string::npos)
            {
                test_campaign_line.replace(pos, 3, "../../../");
            }
            else
            {
                test_campaign_line.replace(pos, 3, "../../");
            }
        }
        
        //7) change export and filter references
        if(((pos = test_campaign_line.find(" extra_filter")) != string::npos) ||
                ((pos = test_campaign_line.find(" app_to_level")) != string::npos) ||
                ((pos = test_campaign_line.find(" level")) != string::npos))
        {
            if((pos = test_campaign_line.find(" extra_filter")) != string::npos)
            {
                if((posEnd = test_campaign_line.substr(pos+15).find("\"")) == string::npos)
                {
                    errorInfo = "extra_filter";
                    status = CANT_FIND_END_QUOTES;
                    return 1;
                }
                uint32_t refNumber;
                istringstream(test_campaign_line.substr(pos+15+1, posEnd)) >> refNumber;
                if(refNumber > references.size())
                {
                    errorInfo = test_campaign_line.substr(pos+15+1, posEnd);
                    status = REFERENCE_TOO_BIG;
                    return 1;
                }
                test_proc_file << test_campaign_line.substr(0, pos) << ">" << endl;
                test_proc_file << "            <extra_filter xsi:type=\"gss_1:GSSFilterMintermFilter\""
                        << " href=\"TMs/" << references.at(refNumber) << ".xmi#/\"/>" << endl;
                
                posEnd = test_campaign_line.find("<level");
                test_proc_file << "          </level"
                        << test_campaign_line.substr(posEnd+6, 1) << "_filter>" << endl;
            }
            bool inputAlreadyWritten = false;
            if((pos = test_campaign_line.find(" app_to_level")) != string::npos)
            {
                if((posEnd = test_campaign_line.substr(pos+16).find("\"")) == string::npos)
                {
                    errorInfo = "app_to_levelX";
                    status = CANT_FIND_END_QUOTES;
                    return 1;
                }
                uint32_t refNumber;
                istringstream(test_campaign_line.substr(pos+16+1, posEnd)) >> refNumber;
                if(refNumber > references.size())
                {
                    errorInfo = test_campaign_line.substr(pos+16+1, posEnd);
                    status = REFERENCE_TOO_BIG;
                    return 1;
                }
                test_proc_file << test_campaign_line.substr(0, pos) << ">" << endl;
                inputAlreadyWritten = true;
                test_proc_file << "          <app_to_level"
                        << test_campaign_line.substr(pos+13, 1)
                        << " href=\"TCs/" << references.at(refNumber) << ".xmi#/\"/>" << endl;
                lastLineWasInputTag = true;
            }
            if((pos = test_campaign_line.find(" level")) != string::npos)
            {
                if((posEnd = test_campaign_line.substr(pos+19).find("\"")) == string::npos)
                {
                    errorInfo = "levelX_to_levelY";
                    status = CANT_FIND_END_QUOTES;
                    return 1;
                }
                uint32_t refNumber;
                istringstream(test_campaign_line.substr(pos+19+1, posEnd)) >> refNumber;
                if(refNumber > references.size())
                {
                    errorInfo = test_campaign_line.substr(pos+19+1, posEnd);
                    status = REFERENCE_TOO_BIG;
                    return 1;
                }
                if(inputAlreadyWritten == false)
                    test_proc_file << test_campaign_line.substr(0, pos) << ">" << endl;
                test_proc_file << "            <" << test_campaign_line.substr(pos+1, 16)
                        << " href=\"TCs/" << references.at(refNumber) << ".xmi#/\"/>" << endl;
                lastLineWasInputTag = true;
            }
        }
        else
        {
            test_proc_file << test_campaign_line << endl;
        }
    }
    campaign_file << "\t</Tests>" << endl;
    campaign_file << "</gss:GSSCampaignCampaign>" << endl;
    return 0;
}