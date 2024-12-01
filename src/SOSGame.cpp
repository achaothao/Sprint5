// A Chao Thao
// Sprint 5
// 12/1/2024

#include "SOSGame.h"
#include <algorithm>
#include <iostream>
#include <sstream> // For std::istringstream



wxBEGIN_EVENT_TABLE(SOSGame, wxFrame)
EVT_LEFT_DOWN(SOSGame::OnCellClicked)
wxEND_EVENT_TABLE()

SOSGame::SOSGame(const wxString& sosgame)
    : wxFrame(NULL, wxID_ANY, sosgame, wxDefaultPosition, wxSize(700, 420)), currentPlayer(0), bluePlayerSOSCount(0), redPlayerSOSCount(0)
{
    // Game pannel
    panel = new wxPanel(this, wxID_ANY);
    panel->SetBackgroundColour(wxColour(255, 255, 255));

    // The rest are game components
    new wxStaticText(panel, wxID_ANY, "SOS", wxPoint(155, 5), wxDefaultSize, wxALIGN_LEFT);
    simpleGameRadio = new wxRadioButton(panel, wxID_ANY, "Simple Game", wxPoint(190, 5), wxDefaultSize, wxRB_GROUP);
    generalGameRadio = new wxRadioButton(panel, wxID_ANY, "General Game", wxPoint(330, 5));

    new wxStaticText(panel, wxID_ANY, "Board Size:", wxPoint(470, 5), wxDefaultSize, wxALIGN_LEFT);
    wxArrayString sizes;
    for (int i = 3; i <= 10; ++i)
        sizes.Add(wxString::Format("%d", i));

    boardSizeChoice = new wxChoice(panel, wxID_ANY, wxPoint(540, 5), wxDefaultSize, sizes);
    boardSizeChoice->SetSelection(0);

    // Blue Player Components
    wxStaticText* bluePlayerLabel = new wxStaticText(panel, wxID_ANY, "Blue Player", wxPoint(10, 50), wxDefaultSize, wxALIGN_LEFT);
    bluePlayerLabel->SetForegroundColour(*wxBLUE);
    bluePlayerHuman = new wxRadioButton(panel, wxID_ANY, "Human", wxPoint(10, 80), wxDefaultSize, wxRB_GROUP);
    bluePlayerComputer = new wxRadioButton(panel, wxID_ANY, "Computer", wxPoint(10, 140));
    bluePlayerS = new wxRadioButton(panel, wxID_ANY, "S", wxPoint(30, 100), wxDefaultSize, wxRB_GROUP);
    bluePlayerO = new wxRadioButton(panel, wxID_ANY, "O", wxPoint(30, 120));

    // Red Player Components
    wxStaticText* redPlayerLabel = new wxStaticText(panel, wxID_ANY, "Red Player", wxPoint(580, 50), wxDefaultSize, wxALIGN_LEFT);
    redPlayerLabel->SetForegroundColour(*wxRED);
    redPlayerHuman = new wxRadioButton(panel, wxID_ANY, "Human", wxPoint(580, 80), wxDefaultSize, wxRB_GROUP);
    redPlayerComputer = new wxRadioButton(panel, wxID_ANY, "Computer", wxPoint(580, 140));
    redPlayerS = new wxRadioButton(panel, wxID_ANY, "S", wxPoint(600, 100), wxDefaultSize, wxRB_GROUP);
    redPlayerO = new wxRadioButton(panel, wxID_ANY, "O", wxPoint(600, 120));

    // Record and Replay Components
    recordGameButton = new wxButton(panel, wxID_ANY, "Record Game", wxPoint(10, 330));
    recordGameButton->Bind(wxEVT_BUTTON, &SOSGame::OnRecordGame, this);
    replayGameButton = new wxButton(panel, wxID_ANY, "Replay", wxPoint(580, 300));
    replayGameButton->Bind(wxEVT_BUTTON, &SOSGame::OnReplayGame, this);

    bluePlayerPoints = new wxStaticText(panel, wxID_ANY, "Blue Points: 0", wxPoint(10, 180), wxDefaultSize, wxALIGN_LEFT);
    redPlayerPoints = new wxStaticText(panel, wxID_ANY, "Red Points: 0", wxPoint(580, 180), wxDefaultSize, wxALIGN_LEFT);

    gameLogic = new SimpleGame(3);
    CreateBoard(3);

    currentTurn = new wxStaticText(panel, wxID_ANY, "Current Turn: Blue", wxPoint(250, 355), wxDefaultSize, wxALIGN_CENTER);

    newGameButton = new wxButton(panel, wxID_ANY, "New Game", wxPoint(580, 330));
    newGameButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { OnNewGame(); });

    boardSizeChoice->Bind(wxEVT_CHOICE, [this](wxCommandEvent&) { OnNewGame(); });
}


// Check to see if the player select simple game mode
bool SOSGame::SimpleGameMode() const
{
    return simpleGameRadio->GetValue();
}

// Check to see if the player select general game mode
bool SOSGame::GeneralGameMode() const
{
    return generalGameRadio->GetValue();
}

// Create game board dimension base on player choice
void SOSGame::CreateBoard(int size)
{
    for (auto& cell : cells)
        cell->Destroy();
    cells.clear();

    int xPosition = 160;
    int yPosition = 50;
    int boxWidth = 35;
    int boxHeight = 30;
    int gap = 34;

    for (int row = 0; row < size; ++row)
    {
        for (int col = 0; col < size; ++col)
        {
            wxPanel* cell = new wxPanel(panel, wxID_ANY, wxPoint(xPosition + (col * gap), yPosition + (row * boxHeight)),
                wxSize(boxWidth, boxHeight), wxSIMPLE_BORDER);
            cells.push_back(cell);
            cell->Bind(wxEVT_LEFT_DOWN, &SOSGame::OnCellClicked, this);
        }
    }
    gameLogic->ResetBoard(size);
    UpdateCurrentTurn();
}

// Handle click events for 'S' & 'O'
void SOSGame::OnCellClicked(wxMouseEvent& event)
{
    wxPanel* cell = dynamic_cast<wxPanel*>(event.GetEventObject());
    if (!cell) return;

    int index = find(cells.begin(), cells.end(), cell) - cells.begin();
    if (index < 0 || index >= cells.size()) return;

    int row = index / gameLogic->GetBoardSize();
    int col = index % gameLogic->GetBoardSize();

    // If player click on occupied cell
    if (gameLogic->IsCellOccupied(row, col))
    {
        wxMessageBox("This cell is already occupied! Please select a different cell.",
            "Invalid Move", wxOK | wxICON_ERROR);
        return;
    }

    char move = (currentPlayer == 0) ? (bluePlayerS->GetValue() ? 'S' : 'O') : (redPlayerS->GetValue() ? 'S' : 'O');

    // Get the player type information for the record file.
    const string playerType = (currentPlayer == 0 && bluePlayerHuman->GetValue()) ||
        (currentPlayer == 1 && redPlayerHuman->GetValue()) ? "Human" : "Computer";
    const string color = (currentPlayer == 0) ? "Blue" : "Red";

    if (gameLogic->PlaceMove(row, col, move))
    {
        wxStaticText* moveText = new wxStaticText(cell, wxID_ANY, wxString::Format("%c", move), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
        const string playerType = (currentPlayer == 0 && bluePlayerHuman->GetValue()) || 
                                   (currentPlayer == 1 && redPlayerHuman->GetValue()) ? "Human" : "Computer";
        const string color = (currentPlayer == 0) ? "Blue" : "Red";
        moveText->SetForegroundColour(currentPlayer == 0 ? *wxBLUE : *wxRED);

        // This file will be created after the game had been record it.
        // This file is save in the project folder.
        if (recordingEnabled && recordFile.is_open())
        {
            recordFile << "PlayerType: " << playerType << ", "
                << "Color: " << color << ", "
                << "MoveType: " << move << ", "
                << "Row: " << row << ", "
                << "Col: " << col << "\n";
        }
        if (currentPlayer == 0) {
            moveText->SetForegroundColour(*wxBLUE);
        }
        else {
            moveText->SetForegroundColour(*wxRED);
        }

        vector<pair<int, int>> sosCells;
        if (gameLogic->CheckForSOS(row, col, move, sosCells))
        {
            if (currentPlayer == 0)
                ++bluePlayerSOSCount;
            else
                ++redPlayerSOSCount;

            UpdatePointsDisplay();

            // Draw a line through SOS that was made by the players
            // Blue line for blue player, red line for red player
            wxColour lineColor = (currentPlayer == 0) ? *wxBLUE : *wxRED;
            for (const auto& cellCoord : sosCells) {
                int cellIndex = cellCoord.first * gameLogic->GetBoardSize() + cellCoord.second;
                wxPanel* sosCell = cells[cellIndex];
                wxClientDC dc(sosCell);
                dc.SetPen(wxPen(lineColor, 2));

                if (sosCells[0].first == sosCells[2].first) {
                    dc.DrawLine(0, sosCell->GetSize().GetHeight() / 2, sosCell->GetSize().GetWidth(), sosCell->GetSize().GetHeight() / 2);
                }
                else if (sosCells[0].second == sosCells[2].second) {
                    dc.DrawLine(sosCell->GetSize().GetWidth() / 2, 0, sosCell->GetSize().GetWidth() / 2, sosCell->GetSize().GetHeight());
                }
                else if (sosCells[0].first < sosCells[2].first && sosCells[0].second < sosCells[2].second) {
                    dc.DrawLine(0, 0, sosCell->GetSize().GetWidth(), sosCell->GetSize().GetHeight());
                }
                else if (sosCells[0].first < sosCells[2].first && sosCells[0].second > sosCells[2].second) {
                    dc.DrawLine(sosCell->GetSize().GetWidth(), 0, 0, sosCell->GetSize().GetHeight());
                }
            }

            // Message display if a player win in a simple game
            if (SimpleGameMode())
            {
                wxString winner = (currentPlayer == 0) ? "Blue" : "Red";
                wxMessageBox(winner + " Wins!", "Congratulations!", wxOK | wxICON_INFORMATION);

                return;
            }

            if (GeneralGameMode())
            {
                UpdateCurrentTurn();
                return;
            }
        }

        // Message display if its a draw in a simple game
        if (SimpleGameMode() && gameLogic->IsBoardFull())
        {
            wxMessageBox("It's a draw!", "Game Over!", wxOK | wxICON_INFORMATION);

            return;
        }
        // Message display if a player win or draw in a general game
        else if (GeneralGameMode() && gameLogic->IsBoardFull())
        {
            if (bluePlayerSOSCount > redPlayerSOSCount)
                wxMessageBox("Blue Wins!", "Congratulations!", wxOK | wxICON_INFORMATION);
            else if (redPlayerSOSCount > bluePlayerSOSCount)
                wxMessageBox("Red Wins!", "Congratulations!", wxOK | wxICON_INFORMATION);
            else
                wxMessageBox("It's a draw!", "Game Over!", wxOK | wxICON_INFORMATION);

            return;
        }

        currentPlayer = (currentPlayer + 1) % 2;
        UpdateCurrentTurn();
        ComputerMove(); // Apply Computer logic 
    }
}

// A message will pop up before and after game record.
void SOSGame::OnRecordGame(wxCommandEvent& event)
{
    if (recordingEnabled) {
        recordingEnabled = false;
        recordFile.close();
        wxMessageBox("Game recording stopped!", "Recording", wxOK | wxICON_INFORMATION);
    }
    else {
        recordFile.open("recorded_game.txt", ios::out | ios::trunc);
        if (!recordFile.is_open()) {
            wxMessageBox("Failed to open file for recording!", "Error", wxOK | wxICON_ERROR);
            return;
        }
        recordingEnabled = true;
        wxMessageBox("Game recording started!", "Recording", wxOK | wxICON_INFORMATION);
    }
}

// Replay the previous game
void SOSGame::ReplayMove(const string& playerType, const string& color, char moveType, int row, int col)
{
    // Determine the current player based on color
    currentPlayer = (color == "Blue") ? 0 : 1;

    // Place the move in the game logic
    gameLogic->PlaceMove(row, col, moveType);

    // Update the UI
    wxPanel* cell = cells[row * gameLogic->GetBoardSize() + col];
    wxStaticText* moveText = new wxStaticText(cell, wxID_ANY, wxString::Format("%c", moveType), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
    moveText->SetForegroundColour(currentPlayer == 0 ? *wxBLUE : *wxRED);

    // Check for SOS formations and update points
    vector<pair<int, int>> sosCells;
    if (gameLogic->CheckForSOS(row, col, moveType, sosCells)) {
        if (currentPlayer == 0) {
            ++bluePlayerSOSCount;
        }
        else {
            ++redPlayerSOSCount;
        }

        UpdatePointsDisplay();

        // Draw SOS lines
        wxColour lineColor = (currentPlayer == 0) ? *wxBLUE : *wxRED;
        for (const auto& cellCoord : sosCells) {
            int cellIndex = cellCoord.first * gameLogic->GetBoardSize() + cellCoord.second;
            wxPanel* sosCell = cells[cellIndex];
            wxClientDC dc(sosCell);
            dc.SetPen(wxPen(lineColor, 2));

            // Draw appropriate lines based on SOS formation
            if (sosCells[0].first == sosCells[2].first) {
                dc.DrawLine(0, sosCell->GetSize().GetHeight() / 2, sosCell->GetSize().GetWidth(), sosCell->GetSize().GetHeight() / 2);
            }
            else if (sosCells[0].second == sosCells[2].second) {
                dc.DrawLine(sosCell->GetSize().GetWidth() / 2, 0, sosCell->GetSize().GetWidth() / 2, sosCell->GetSize().GetHeight());
            }
            else if (sosCells[0].first < sosCells[2].first && sosCells[0].second < sosCells[2].second) {
                dc.DrawLine(0, 0, sosCell->GetSize().GetWidth(), sosCell->GetSize().GetHeight());
            }
            else if (sosCells[0].first < sosCells[2].first && sosCells[0].second > sosCells[2].second) {
                dc.DrawLine(sosCell->GetSize().GetWidth(), 0, 0, sosCell->GetSize().GetHeight());
            }
        }
    }

    // Update the turn display
    UpdateCurrentTurn();
}


void SOSGame::OnReplayGame(wxCommandEvent& event)
{
    ifstream replayFile("recorded_game.txt");
    if (!replayFile.is_open()) {
        wxMessageBox("No recorded game found!", "Error", wxOK | wxICON_ERROR);
        return;
    }

    OnNewGame(); // Reset the game

    string line;
    while (getline(replayFile, line)) {
        istringstream iss(line);

        string playerType, color;
        char moveType;
        int row, col;

        // Parse the recorded line
        string key, value;
        while (iss >> key >> value) {
            if (key == "PlayerType:") {
                playerType = value;
            }
            else if (key == "Color:") {
                color = value;
            }
            else if (key == "MoveType:") {
                moveType = value[0]; 
            }
            else if (key == "Row:") {
                row = stoi(value);
            }
            else if (key == "Col:") {
                col = stoi(value);
            }
        }

        ReplayMove(playerType, color, moveType, row, col);
    }

    replayFile.close();
    wxMessageBox("Replay complete!", "Replay", wxOK | wxICON_INFORMATION);
}

string SOSGame::GetPlayerType() const
{
    return (currentPlayer == 0) ?
        (bluePlayerHuman->GetValue() ? "Human" : "Computer") :
        (redPlayerHuman->GetValue() ? "Human" : "Computer");
}

string SOSGame::GetPlayerColor() const
{
    return currentPlayer == 0 ? "Blue" : "Red";
}

// This Function get call when Blue player or Red player choose computer
void SOSGame::ComputerMove()
{
    // Check if the current player is set to Computer
    bool isComputerTurn = (currentPlayer == 0 && bluePlayerComputer->GetValue()) ||
        (currentPlayer == 1 && redPlayerComputer->GetValue());

    if (!isComputerTurn) return;

    // Select 'S' or 'O' randomly
    char move = (rand() % 2 == 0) ? 'S' : 'O';

    // Gather all empty cells
    int boardSize = gameLogic->GetBoardSize();
    vector<pair<int, int>> emptyCells;

    for (int row = 0; row < boardSize; ++row) {
        for (int col = 0; col < boardSize; ++col) {
            if (!gameLogic->IsCellOccupied(row, col)) {
                emptyCells.emplace_back(row, col);
            }
        }
    }

    // If no empty cells are available, return
    if (emptyCells.empty()) return;

    // Choose a random empty cell
    int randomIndex = rand() % emptyCells.size();
    int row = emptyCells[randomIndex].first;
    int col = emptyCells[randomIndex].second;

    // Place the move in the chosen cell
    if (gameLogic->PlaceMove(row, col, move))
    {
        wxPanel* cell = cells[row * boardSize + col];
        wxStaticText* moveText = new wxStaticText(cell, wxID_ANY, wxString::Format("%c", move), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER);
        moveText->SetForegroundColour(currentPlayer == 0 ? *wxBLUE : *wxRED);

        // Record the computer's move
        if (recordingEnabled && recordFile.is_open()) {
            const string playerType = "Computer";
            const string color = (currentPlayer == 0) ? "Blue" : "Red";
            recordFile << "PlayerType: " << playerType << ", "
                << "Color: " << color << ", "
                << "MoveType: " << move << ", "
                << "Row: " << row << ", "
                << "Col: " << col << "\n";
        }

        vector<pair<int, int>> sosCells;
        if (gameLogic->CheckForSOS(row, col, move, sosCells))
        {
            if (currentPlayer == 0)
                ++bluePlayerSOSCount;
            else
                ++redPlayerSOSCount;

            UpdatePointsDisplay();

            // Draw a line through the SOS that was made by the players
            wxColour lineColor = (currentPlayer == 0) ? *wxBLUE : *wxRED;
            for (const auto& cellCoord : sosCells) {
                int cellIndex = cellCoord.first * gameLogic->GetBoardSize() + cellCoord.second;
                wxPanel* sosCell = cells[cellIndex];
                wxClientDC dc(sosCell);
                dc.SetPen(wxPen(lineColor, 2));

                if (sosCells[0].first == sosCells[2].first) {
                    dc.DrawLine(0, sosCell->GetSize().GetHeight() / 2, sosCell->GetSize().GetWidth(), sosCell->GetSize().GetHeight() / 2);
                }
                else if (sosCells[0].second == sosCells[2].second) {
                    dc.DrawLine(sosCell->GetSize().GetWidth() / 2, 0, sosCell->GetSize().GetWidth() / 2, sosCell->GetSize().GetHeight());
                }
                else if (sosCells[0].first < sosCells[2].first && sosCells[0].second < sosCells[2].second) {
                    dc.DrawLine(0, 0, sosCell->GetSize().GetWidth(), sosCell->GetSize().GetHeight());
                }
                else if (sosCells[0].first < sosCells[2].first && sosCells[0].second > sosCells[2].second) {
                    dc.DrawLine(sosCell->GetSize().GetWidth(), 0, 0, sosCell->GetSize().GetHeight());
                }
            }

            if (SimpleGameMode())
            {
                wxString winner = (currentPlayer == 0) ? "Blue" : "Red";
                wxMessageBox(winner + " Wins!", "Congratulations!", wxOK | wxICON_INFORMATION);
                return;
            }

            if (GeneralGameMode())
            {
                ComputerMove(); // Computer goes again if it formed an SOS in General Game Mode
                return;
            }
        }

        // Check for draw or win conditions
        if (SimpleGameMode() && gameLogic->IsBoardFull())
        {
            wxMessageBox("It's a draw!", "Game Over!", wxOK | wxICON_INFORMATION);
            return;
        }
        else if (GeneralGameMode() && gameLogic->IsBoardFull())
        {
            if (bluePlayerSOSCount > redPlayerSOSCount)
                wxMessageBox("Blue Wins!", "Congratulations!", wxOK | wxICON_INFORMATION);
            else if (redPlayerSOSCount > bluePlayerSOSCount)
                wxMessageBox("Red Wins!", "Congratulations!", wxOK | wxICON_INFORMATION);
            else
                wxMessageBox("It's a draw!", "Game Over!", wxOK | wxICON_INFORMATION);
            return;
        }

        // Change turn to the other player if no SOS was formed
        currentPlayer = (currentPlayer + 1) % 2;
        UpdateCurrentTurn();

        // Check if the new turn is also for a computer
        ComputerMove();
    }
}

// Game restart for new game
void SOSGame::OnNewGame()
{
    delete gameLogic;
    int boardSize = boardSizeChoice->GetSelection() + 3;
    gameLogic = SimpleGameMode() ? static_cast<SOSGameLogic*>(new SimpleGame(boardSize))
        : static_cast<SOSGameLogic*>(new GeneralGame(boardSize));

    CreateBoard(boardSize);
    ResetPlayerSelections();
    currentPlayer = 0;
    bluePlayerSOSCount = 0;
    redPlayerSOSCount = 0;
    UpdatePointsDisplay();
    UpdateCurrentTurn();

    // Start the game with a computer move if the blue player is set to Computer
    if (bluePlayerComputer->GetValue()) {
        ComputerMove();
    }
}

// Show players point
void SOSGame::UpdatePointsDisplay()
{
    bluePlayerPoints->SetLabel(wxString::Format("Blue Points: %d", bluePlayerSOSCount));
    redPlayerPoints->SetLabel(wxString::Format("Red Points: %d", redPlayerSOSCount));
}

void SOSGame::ResetPlayerSelections()
{
    bluePlayerS->SetValue(true);
    redPlayerS->SetValue(true);
}

// Show which player making the next move
void SOSGame::UpdateCurrentTurn()
{
    if (!currentTurn) {
        return;
    }
    wxString turn = (currentPlayer == 0) ? "Current Turn: Blue" : "Current Turn: Red";
    currentTurn->SetLabel(turn);
}

class SOSApp : public wxApp
{
public:
    virtual bool OnInit();
};

IMPLEMENT_APP(SOSApp)

bool SOSApp::OnInit()
{
    SOSGame* game = new SOSGame("SOS Game");
    game->Show(true);
    return true;
}