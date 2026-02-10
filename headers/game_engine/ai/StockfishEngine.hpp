#pragma once

#include <cstdio>
#include <optional>
#include <string>
#include <vector>

class StockfishEngine
{
public:
    StockfishEngine() = default;
    ~StockfishEngine();

    StockfishEngine(const StockfishEngine&) = delete;
    StockfishEngine& operator=(const StockfishEngine&) = delete;

    bool start(const std::string& path = "stockfish");
    void stop();

    std::optional<std::string> bestMoveUci(const std::vector<std::string>& moves_uci, int depth);

private:
    bool sendCommand(const std::string& command);
    bool waitForToken(const std::string& token);

private:
    int child_pid_{-1};
    FILE* to_engine_{nullptr};
    FILE* from_engine_{nullptr};
};
