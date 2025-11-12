#ifndef INQUISITOR_FACADE_HPP
#define INQUISITOR_FACADE_HPP

#include <string>
#include <memory>

class ArpPoisoner;
class PacketCapture;
class FtpParser;

class Inquisitor
{
public:
	struct Config
	{
		std::string ipSrc;
		std::string macSrc;
		std::string ipTarget;
		std::string macTarget;
		bool verbose;
		std::string interface;
	};
	Inquisitor(const Config &config);
	~Inquisitor();

	bool initialize();
	void startAttack();
	void stopAttack();

	Inquisitor(const Inquisitor &) = delete;
	Inquisitor &operator=(const Inquisitor &) = delete;

private:
	Config config_;
	std::unique_ptr<ArpPoisoner> ArpPoisoner_;
	std::unique_ptr<PacketCapture> PacketCapture_;
	std::unique_ptr<FtpParser> ftpParser_;
	bool isRunning_;

	void setupSignalHandlers();
	void restoreArptTables();
};

#endif