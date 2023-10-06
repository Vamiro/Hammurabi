#include <iostream>
#include <fstream>
#include <random>
#include <sstream>

int GenerateRandomNumber(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(min, max);
    return dist(gen);
}

int GetIntFromInput()
{
    std::string line;
    std::size_t pos;
    int i;

    if (!std::getline(std::cin, line)) {
        throw std::runtime_error("unexpected input error!\n");
    }

    try {
        i = std::stoi(line, &pos);
    } catch (std::invalid_argument&) {
        throw std::exception("Unable to convert input to number, try again!\n");
    } catch (std::out_of_range&) {
        throw std::exception("Out of range error, try again!\n");
    }

    for (; pos < line.length(); pos++) {
        if (!std::isspace(unsigned char(line[pos]))) {
            throw std::exception("Invalid character found, try again!\n");
        }
    }
    return i;
}

//--------------------------------------------------------------------------------- Класс города
class Town {
private:
    int population_; // Текущее население города
    int deaths_; // Количество людей, умерших от голода
    float averageDeaths_; // Среднегодовой процент умерших от голода 
    int percentageDeaths_; // Процент умерших от голода в год
    int immigrants_; // Количество людей, приехавших в город
    bool plague_; // Флаг наличия чумы

    int wheatInStorage_; // Сколько всего пшеницы
    int wheatHarvested_; // Сколько всего пшеницы было собрано
    int wheatSowed_; // Сколько пшеницы было засеяно
    int wheatPerAcre_; // Сколько пшеницы было получено с акра
    int wheatEatenByRats_; // Сколько пшеницы уничтожили крысы

    int ownedAcres_; // Сколько акров сейчас занимает город
    int acrePrice_; // Какова цена одного акра земли в этом 

public:
    Town();
    bool UpdatePopulation(int wheatToUse);
    void CheckForPlague();

    bool BuyAcres(int acres);
    bool SellAcres(int acres);
    void CalcAcrePrice();

    void HarvestCrops();
    bool SowCrops(int acresToSow);
    void HandleRatInfestation();

    int GetPopulation() { return population_; }
    int GetDeaths() { return deaths_; }
    float GetAverageDeaths() { return averageDeaths_; }
    int GetPercentageDeaths() { return percentageDeaths_; }
    int GetImmigrants() { return immigrants_; }
    bool IsPlague() { return plague_; }

    int GetWheatInStorage() { return wheatInStorage_; }
    int GetWheatHarvested() { return wheatHarvested_; }
    int GetWheatPerAcre() { return wheatPerAcre_; }
    int GetWheatEatenByRats() { return wheatEatenByRats_; }

    int GetOwnedAcres() { return ownedAcres_; }
    int GetAcrePrice() { return acrePrice_; }
};

Town::Town() :  population_(100),
                deaths_(0), percentageDeaths_(0), averageDeaths_(0), immigrants_(0), plague_(false),
                wheatInStorage_(2800), wheatHarvested_(0), wheatPerAcre_(0), wheatEatenByRats_(0), wheatSowed_(0),
                ownedAcres_(1000), acrePrice_(GenerateRandomNumber(17, 26)) {
}

bool Town::UpdatePopulation(int wheatToUse) {
    if (wheatToUse < 0 || wheatToUse > wheatInStorage_) return false;

    // Рассчитываем смертность населения из-за голода
    deaths_ = population_ - wheatToUse / 20;
    wheatInStorage_ -= wheatToUse;

    if (deaths_ <= 0) {
        deaths_ = 0;
    } else {
        averageDeaths_ += float(deaths_) / population_;
        percentageDeaths_ = float(deaths_) / population_ * 100;
        population_ -= deaths_;
    }

    // Рассчитываем иммигрантов
    immigrants_ = (deaths_ / 2 + (5 - wheatPerAcre_) * wheatInStorage_ / 600 + 1);

    if (immigrants_ < 0) {
        immigrants_ = 0;
    } else if (immigrants_ > 50) {
        immigrants_ = 50;
    }
    population_ += immigrants_;

    return true;
}

bool Town::BuyAcres(int acres) {
    int cost = acres * acrePrice_;

    if (cost > wheatInStorage_ || acres < 0) return false;
    ownedAcres_ += acres;
    wheatInStorage_ -= cost;

    return true;
}

bool Town::SellAcres(int acres) {
    if (acres > ownedAcres_ || acres < 0) return false;
    int profit = acres * acrePrice_;
    ownedAcres_ -= acres;
    wheatInStorage_ += profit;

    return true;

}

void Town::CalcAcrePrice() {
    acrePrice_ = GenerateRandomNumber(17, 26);
}

void Town::HarvestCrops() {
    wheatPerAcre_ = GenerateRandomNumber(1, 6);
    wheatHarvested_ = wheatPerAcre_ * wheatSowed_;
    wheatInStorage_ += wheatHarvested_;
}

bool Town::SowCrops(int acresToSow) {
    if (acresToSow > population_ * 10 || acresToSow > wheatInStorage_ * 2 || acresToSow < 0) return false;
    wheatInStorage_ -= acresToSow / 2;
    wheatSowed_ = acresToSow;
    return true;
}

void Town::HandleRatInfestation() {
    wheatEatenByRats_ = int((GenerateRandomNumber(0, 7) / 100.0) * wheatInStorage_);
    wheatInStorage_ -= wheatEatenByRats_;
}

void Town::CheckForPlague() {
    double chanceOfPlague = 0.15;
    if (GenerateRandomNumber(1, 100) <= (chanceOfPlague * 100)) {
        population_ /= 2;
        plague_ = true;
    } else {
        plague_ = false;
    }
}

//--------------------------------------------------------------------------------- Игровой цикл
class Game {
private:
    Town town_;
    int roundNumber_; // Номер текущего раунда
public:
    Game();
    void StartGame();
    void LoadGame();
    bool QuitGame();
    void SaveGame();
    
    void PlayRound();
    void PlayerInput(bool(Town::* ptr) (int), const char* output);
    void DisplayStatistics();

    bool IsGameOver();
};
    
Game::Game() : roundNumber_(1){}

void Game::StartGame() {
    LoadGame();
    //Проверка на файл сохранения с возможностью загрузки
    while (!IsGameOver()) {
        DisplayStatistics();
        if (QuitGame()) break; //Возможность прервать игру с сохранение прогресса
        DisplayStatistics();
        PlayRound();
    }
}

void Game::LoadGame() {
    std::ifstream file;
    file.open("save.dat", std::ios::binary);

    if (file.is_open()) {
        char c;

        do {
            std::cout << "Do you want to load the game? Y/N: ";
            std::cin >> c;

            if (c == 'N' || c == 'n') {
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                return;
            }
        } while (c != 'Y' && c != 'y');

        file.read(reinterpret_cast<char*>(this), sizeof(*this));
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Game loaded." << std::endl;
    }
}

bool Game::QuitGame() {
    std::cout << "Do you want to save and quit the game? Enter to continue / Q to quit: ";
    char c;
    std::cin >> std::noskipws >> c;

    if (c == 'Q' || c == 'q') {
        SaveGame();
        return true;
    }

    if(c != '\n') std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return false;
}

void Game::SaveGame() {
    std::ofstream file;
    file.open("save.dat", std::ios::binary);
    if (file.is_open()) {
        file.write(reinterpret_cast<char*>(this), sizeof(*this));
        std::cout << "Game saved." << std::endl;
    }
}

void Game::PlayRound() {
    PlayerInput(&Town::BuyAcres, "Enter the number of acres to buy: ");
    DisplayStatistics();

    PlayerInput(&Town::SellAcres, "Enter the number of acres to sell: ");
    DisplayStatistics();

    PlayerInput(&Town::SowCrops, "Enter the number of acres to sow: ");
    DisplayStatistics();

    PlayerInput(&Town::UpdatePopulation, "Enter the amount of wheat to use: ");
    town_.HandleRatInfestation();
    town_.CheckForPlague();

    town_.HarvestCrops();
    town_.CalcAcrePrice();
    
    roundNumber_++;
}

void Game::PlayerInput(bool(Town::* ptr) (int), const char* output) {
    bool success = false;
    do {
        int inputInt;
        std::cout << output;

        try {
            inputInt = GetIntFromInput();
            success = (town_.*ptr)(inputInt);
            if (!success) {
                std::cout << "Wrong number to use." << std::endl;
            }
        }
        catch (std::exception e) {
            std::cout << "Invalid input." << std::endl;
        }
    } while (!success);
}

void Game::DisplayStatistics() {
    // Очистка интерфейса
    system("cls");
    // Вывод информации о текущем раунде
    std::cout << "--------------------------------------------------------------------" << std::endl;
    std::cout << "Year " << (roundNumber_ > 10 ? NULL : roundNumber_) << std::endl;
    std::cout << "--------------------------------------------------------------------" << std::endl;

    // Вывод информации о населении
    if(town_.GetDeaths() > 0) std::cout << "Deaths out of hunger:\t" << town_.GetDeaths() << std::endl;
    if (town_.GetImmigrants() > 0) std::cout << "Immigrants this year:\t" << town_.GetImmigrants() << std::endl;
    if(town_.IsPlague()) std::cout << "There was a plague in the town and the population decreased in half." << std::endl;
    std::cout << "Total population:\t" << town_.GetPopulation() << std::endl;
    
    // Вывод информации об урожае
    std::cout << "--------------------------------------------------------------------" << std::endl;
    std::cout << "Wheat harvested:\t" << town_.GetWheatHarvested() << "\tper an acre:\t" << town_.GetWheatPerAcre() << std::endl;
    std::cout << "Wheat eaten by rats:\t" << town_.GetWheatEatenByRats() << std::endl;
    std::cout << "Wheat in storage:\t" << town_.GetWheatInStorage() << std::endl;

    // Вывод информации о землях
    std::cout << "--------------------------------------------------------------------" << std::endl;
    std::cout << "Owned acres:\t\t" << town_.GetOwnedAcres() << std::endl;
    std::cout << "Price of an acre\t" << town_.GetAcrePrice() << std::endl;
    std::cout << "--------------------------------------------------------------------" << std::endl << std::endl;
}

bool Game::IsGameOver() {
    DisplayStatistics();
    if (town_.GetPercentageDeaths() >= 45) {
        std::cout << "Game Over: More than 45% of the population died from hunger." << std::endl;
        return true;
    } else if (roundNumber_ > 10) {
        std::cout << "Hooray! You completed 10 years of your governance!" << std::endl;
        int averageDeaths = std::ceil(town_.GetAverageDeaths() / 10 * 100);
        int acresPerInhabitant = town_.GetOwnedAcres() / town_.GetPopulation();
        std::cout << "Average deaths: " << averageDeaths << "%\tAcres per inhabitant: " << acresPerInhabitant << std::endl;

        if (averageDeaths > 33 || acresPerInhabitant < 7) {
            std::cout << "Very bad!!!";
        }
        else if (averageDeaths > 10 || acresPerInhabitant < 9) {
            std::cout << "Fifty Fifty.";
        }
        else if (averageDeaths > 3 || acresPerInhabitant < 10) {
            std::cout << "Good!";
        }
        else {
            std::cout << "Excelente!!!";
        }
        return true;
    }
    return false;
}

int main() {
    Game game;
    game.StartGame();
    return 0;
}