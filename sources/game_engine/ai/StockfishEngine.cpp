#include "game_engine/ai/StockfishEngine.hpp"

#include <array>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <sstream>
#include <string>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

StockfishEngine::~StockfishEngine()
{
    stop();
}

bool StockfishEngine::start(const std::string& path)
{
    stop();

    int to_child_pipe[2]{};
    int from_child_pipe[2]{};

    if (pipe(to_child_pipe) != 0) return false;
    if (pipe(from_child_pipe) != 0)
    {
        close(to_child_pipe[0]);
        close(to_child_pipe[1]);
        return false;
    }

    const pid_t pid = fork();
    if (pid < 0)
    {
        close(to_child_pipe[0]);
        close(to_child_pipe[1]);
        close(from_child_pipe[0]);
        close(from_child_pipe[1]);
        return false;
    }

    if (pid == 0)
    {
        dup2(to_child_pipe[0], STDIN_FILENO);
        dup2(from_child_pipe[1], STDOUT_FILENO);
        dup2(from_child_pipe[1], STDERR_FILENO);

        close(to_child_pipe[0]);
        close(to_child_pipe[1]);
        close(from_child_pipe[0]);
        close(from_child_pipe[1]);

        execlp(path.c_str(), path.c_str(), static_cast<char*>(nullptr));
        _exit(127);
    }

    close(to_child_pipe[0]);
    close(from_child_pipe[1]);

    FILE* to_engine = fdopen(to_child_pipe[1], "w");
    FILE* from_engine = fdopen(from_child_pipe[0], "r");

    if (!to_engine || !from_engine)
    {
        if (to_engine) fclose(to_engine); else close(to_child_pipe[1]);
        if (from_engine) fclose(from_engine); else close(from_child_pipe[0]);
        kill(pid, SIGTERM);
        waitpid(pid, nullptr, 0);
        return false;
    }

    setvbuf(to_engine, nullptr, _IOLBF, 0);

    child_pid_ = static_cast<int>(pid);
    to_engine_ = to_engine;
    from_engine_ = from_engine;

    if (!sendCommand("uci"))
    {
        stop();
        return false;
    }

    if (!waitForToken("uciok"))
    {
        stop();
        return false;
    }

    if (!sendCommand("isready"))
    {
        stop();
        return false;
    }

    if (!waitForToken("readyok"))
    {
        stop();
        return false;
    }

    return true;
}

void StockfishEngine::stop()
{
    if (to_engine_)
    {
        sendCommand("quit");
        fclose(to_engine_);
        to_engine_ = nullptr;
    }

    if (from_engine_)
    {
        fclose(from_engine_);
        from_engine_ = nullptr;
    }

    if (child_pid_ > 0)
    {
        int status = 0;
        const pid_t pid = static_cast<pid_t>(child_pid_);

        if (waitpid(pid, &status, WNOHANG) == 0)
        {
            kill(pid, SIGTERM);
            waitpid(pid, &status, 0);
        }

        child_pid_ = -1;
    }
}

std::optional<std::string> StockfishEngine::bestMoveUci(
    const std::vector<std::string>& moves_uci,
    int depth)
{
    if (!to_engine_ || !from_engine_) return std::nullopt;
    if (depth <= 0) depth = 1;

    if (!sendCommand("ucinewgame")) return std::nullopt;
    if (!sendCommand("isready")) return std::nullopt;
    if (!waitForToken("readyok")) return std::nullopt;

    std::ostringstream position;
    position << "position startpos";
    if (!moves_uci.empty())
    {
        position << " moves";
        for (const auto& move : moves_uci)
            position << ' ' << move;
    }

    if (!sendCommand(position.str())) return std::nullopt;

    std::ostringstream go;
    go << "go depth " << depth;
    if (!sendCommand(go.str())) return std::nullopt;

    std::array<char, 512> buffer{};
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), from_engine_) != nullptr)
    {
        std::string line(buffer.data());

        if (line.rfind("bestmove", 0) == 0)
        {
            std::istringstream iss(line);
            std::string token;
            std::string move;
            iss >> token >> move;
            if (!move.empty() && move != "(none)") return move;
            return std::nullopt;
        }
    }

    return std::nullopt;
}

bool StockfishEngine::sendCommand(const std::string& command)
{
    if (!to_engine_) return false;
    if (std::fputs(command.c_str(), to_engine_) < 0) return false;
    if (std::fputc('\n', to_engine_) == EOF) return false;
    return std::fflush(to_engine_) == 0;
}

bool StockfishEngine::waitForToken(const std::string& token)
{
    if (!from_engine_) return false;

    std::array<char, 512> buffer{};
    while (fgets(buffer.data(), static_cast<int>(buffer.size()), from_engine_) != nullptr)
    {
        std::string line(buffer.data());
        if (line.rfind(token, 0) == 0) return true;
    }

    return false;
}
