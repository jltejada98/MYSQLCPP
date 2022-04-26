#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <mysql/jdbc.h>

#define URL "localhost"
#define USER "root"
#define PASSWORD "z5QaqQhe"
#define DATABASE "Krymino"

int main() {
    sql::Driver *newDriver = get_driver_instance();

    sql::Connection * connectionPTR = newDriver->connect(URL,USER,PASSWORD );

    connectionPTR->setAutoCommit(false); //Remove auto commit

    connectionPTR->setSchema(DATABASE);

    sql::Statement *queryStatment = connectionPTR->createStatement();

    sql::ResultSet *queryResult;

    //Assume BatchID passed
    std::string batchID = "1";

    //ProteinIDs
    std::vector<std::string> proteinIDs= {"P02144", "P69892"}; //Uniprot IDs
    std::vector<std::string> proteinSequences;
    for(const auto& protein : proteinIDs){
        sql::SQLString proteinQuery = "SELECT * FROM Protein where uniprotID=\"" + protein + "\"";
        queryResult = queryStatment->executeQuery(proteinQuery);
        if(queryResult->next()){
            sql::SQLString seqString = queryResult->getString("Original_Sequence");
            std::cout << seqString << std::endl;
            proteinSequences.push_back(seqString);
        }
        else{
            std::cout << "Result set empty on: " << protein << std::endl;
        }
    }


    //Similarity groups.
    std::vector<std::string> similarityGroupIDs = {"1"}; //IDs = Ints, Only one ID
    std::string similarityString;
    for (const auto & similarityGroupID: similarityGroupIDs) {
        sql::SQLString similarityGroupQuery = "SELECT * FROM Similarity_Group where SimilarityID=\"" +similarityGroupID+ "\"";
        queryResult = queryStatment->executeQuery(similarityGroupQuery);
        if(queryResult->next()){
            sql::SQLString seqString = queryResult->getString("Similarity_String");
            std::cout << seqString << std::endl;
            similarityString = seqString;
        }
        else{
            std::cout << "Result set empty on: " << similarityGroupID  << std::endl;
        }
    }


    //Setup SimilarityGroups Map
    // ASSUME GROUPS ENTERED CONTAIN ALL CHARACTERS.
    int groupIndex = 0;
    std::vector<char> standarizingChars = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};
    std::unordered_map<char, char> similarityCharsMap; //Asume 1 char at one standard position.
    std::pair<char, char> similarityCharPair;
    int i = 1;
    while(i < (similarityString.length()-1)) { //Excludes outside '[' ']'
        if(similarityString[i] == '['){
            ++groupIndex;
            ++i;
        }
        else if((similarityString[i] == '\'') && (similarityString[i+2] == '\'')){ //Get Character in middle
            similarityCharPair = std::make_pair(similarityString[i+1], standarizingChars[groupIndex]);
            similarityCharsMap.insert(similarityCharPair);
            i += 3;
        }
        else{
            ++i;
        }
    }

    //Convert Protein to standarized based on similarity groups
    std::vector<std::string> standarizedProteins;
    for(auto &sequence : proteinSequences){
        std::string newProteinString = "";
        for(auto &seqChar : sequence){
            newProteinString+= similarityCharsMap.at(seqChar);
        }
        standarizedProteins.push_back(newProteinString);
    }

    //Todo, Consider change set in matches data structure.

    //Determine matches in standarized


    //Insert matches into DB
    std::unordered_map<std::string, std::vector<std::unordered_set<int>>> matchesMap;
    std::vector<std::unordered_set<int>> matchLocations;
    std::unordered_set<int> matchIndecies({1,2,3});
    matchLocations.push_back(matchIndecies);
    matchIndecies={4,5,6};
    matchLocations.push_back(matchIndecies);
    std::pair<std::string, std::vector<std::unordered_set<int>>> matchPair(std::make_pair("QQQQQ", matchLocations));
    matchesMap.insert(matchPair);

    matchLocations.clear();
    matchIndecies={7,8,9};
    matchLocations.push_back(matchIndecies);
    matchIndecies={10,11,12};
    matchLocations.push_back(matchIndecies);
    matchPair = std::make_pair("WWWWWW", matchLocations);
    matchesMap.insert(matchPair);



    int insertedID = 0;
    for (auto &match: matchesMap){
        //Perform Insertion into Standardized_Sequence table.
        sql::SQLString matchCheck = "SELECT EXISTS (SELECT * FROM Standardized_Sequence where Sequence_String=\"" + match.first + "\")";
        queryResult = queryStatment->executeQuery(matchCheck);
        if(queryResult->next()){
            bool matchExists = queryResult->getBoolean(1);
            if(matchExists){
                std::cout << "Match" << match.first <<  " already exists." << std::endl;
                sql::SQLString getMatchID = "SELECT Stan_PatternID FROM Standardized_Sequence where Sequence_String=\"" + match.first + "\""; //GET ID if exists.
                queryResult = queryStatment->executeQuery(getMatchID);
                if(queryResult->next()){

                }
                else{

                }

            }
            else{
                std::cout << "Match" << match.first << " doesn't exist." << std::endl;
                sql::SQLString insertMatch = "INSERT INTO Standardized_Sequence (Sequence_String,Length) VALUES ('" + match.first + "','" + std::to_string(match.first.length()) + "')";
                int rows = queryStatment->executeUpdate(insertMatch);
                std::cout << "Update Return "<< rows << std::endl;
                if(rows == 1){
                    std::cout << "Insertion Sucessful" << std::endl;
                    connectionPTR->commit();
                    sql::SQLString getID = "SELECT @@identity"; //"SELECT @@identity AS id"
                    queryResult = queryStatment->executeQuery(getID);
                    if(queryResult->next()){
                        insertedID = queryResult->getInt(1);
                    }
                    else{
                        std::cout << "Get ID not Sucessful" <<std::endl;
                    }
                }
                else{
                    std::cout << "Insertion not Sucessful" <<std::endl;
                }
            }
        }
        else{
            std::cout << "Result set empty on: " << match.first  << std::endl;
        }
        //Perform Insertion of middle-man table
        for (int j = 0; j < proteinIDs.size(); ++j) {
            //Create Location String
            std::string locationString = "";
            for(auto &elem: match.second.at(j)){
                locationString += std::to_string(elem);
                locationString += ", ";
            }
            sql::SQLString insertBatch = "INSERT INTO Batch_Details(BatchID,UniprotID,Stan_PatternID,Location) VALUES ('" + batchID + "','" + proteinIDs[j] + "','" + std::to_string(insertedID) + "','"+ locationString + "')";
            std::cout << insertBatch << std::endl;
            int rows = queryStatment->executeUpdate(insertBatch);
            std::cout << "Update Return "<< rows << std::endl;
            if(rows == 1){
                std::cout << "Insertion Sucessful" << std::endl;
                connectionPTR->commit();
            }
            else{
                std::cout << "Insertion not Sucessful" <<std::endl;
            }
        }
    }

    //Insert into middle-man table



    //Terminate connection


    return 0;
}
