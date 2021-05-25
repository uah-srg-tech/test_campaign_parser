/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   test_campaign_parser.h
 * Author: user
 *
 * Created on 30 de julio de 2019, 18:05
 */

#include <libxml/xmlreader.h>
#include <cstdint>
#include <string>
#include <fstream>

#ifndef TEST_CAMPAIGN_PARSER_H
#define TEST_CAMPAIGN_PARSER_H

class test_campaign_parser {
    
public:
    test_campaign_parser(const char * config_filename_c_str);
    uint32_t createCampaignFromFullCampaign(uint32_t & proceduresParsedRef,
        std::string &envScenario);
    std::string displayCreateCampaignFromFullCampaignError();
    
private:
    uint32_t parseTestCampaignFile(uint32_t & proceduresParsed);
    void checkScenarioInEnvironment(xmlDocPtr doc);
    
    enum Status : uint32_t {
        NO_ERROR,
        CANT_OPEN_CONFIG_FILE,
        CANT_CREATE_CAMPAIGN_FILE,
        FILE_NOT_FOUND,
        FILE_PARSING_ERROR,
        XML_PARSING_ERROR,
        SCENARIO_NOT_FOUND,
        FILE_NOT_VALID,
        CANT_CREATE_DIR,
        CANT_FIND_NAME,
        CANT_FIND_END_QUOTES,
        CANT_CHANGE_DIRECTORY,
        CANT_CREATE_TEST_PROC_FILE,
        TEST_PROC_FILE_NOT_CLOSED,
        TEST_PROC_FILE_NOT_OPENED,
        EXPORT_FILTER_FILE_NOT_OPENED,
        EXPORT_FILTER_FILE_NOT_CLOSED,
        CANT_FIND_EXPORT_FILTER_TYPE,
        CANT_CREATE_EXPORT_FILTER_FILE,
        EXPORT_FILTER_WRONG_TAG,
        REFERENCE_TOO_BIG
    };
    Status status;
    
    std::string config_filename;
    std::string test_campaign_filename;
    std::string test_campaign_name;
    std::string output_filename;
    std::ofstream campaign_file;
    std::ofstream test_proc_file;
    
    uint32_t current_line;
    std::string errorInfo;
    uint32_t xmlParsingStatus;
    
    std::string environment;
    std::string scenario;
    int32_t scenarioRef;
};

#endif /* TEST_CAMPAIGN_PARSER_H */