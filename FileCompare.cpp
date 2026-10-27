#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <system_error>
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = std::filesystem;

namespace {
	// Streaming FNV-1a 64-bit hash: portable and does not require platform libraries.
	std::string hashFile(const fs::path &path) {
		std::ifstream input(path, std::ios::binary);
		if(!input) throw std::runtime_error("ファイルを開けません: " + path.string());
		std::uint64_t hash = 14695981039346656037ull;
		std::array<char, 64 * 1024> buffer{};
		while(input) {
			input.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
			const auto count = input.gcount();
			for(std::streamsize i = 0; i < count; ++i) {
				hash ^= static_cast<unsigned char>(buffer[static_cast<std::size_t>(i)]);
				hash *= 1099511628211ull;
			}
		}
		if(!input.eof()) throw std::runtime_error("ファイルの読み込みに失敗しました: " + path.string());
		std::ostringstream result;
		result << std::hex << std::setfill('0') << std::setw(16) << hash;
		return result.str();
	}

	bool isDirectory(const fs::path &path) {
		std::error_code ec;
		const bool result = fs::is_directory(path, ec);
		if(ec) throw std::runtime_error("パスを確認できません: " + path.string() + ": " + ec.message());
		return result;
	}

	void compareFiles(const fs::path &left, const fs::path &right) {
		if(hashFile(left) == hashFile(right)) {
			std::cout << "一致: " << left.string() << " == " << right.string() << '\n';
		} else {
			std::cout << "相違: " << left.string() << " | " << right.string() << '\n';
		}
	}

	using EntryMap = std::map<fs::path, fs::directory_entry>;
	EntryMap listDirectory(const fs::path &root) {
		EntryMap entries;
		std::error_code ec;
		fs::directory_iterator it(root, fs::directory_options::none, ec), end;
		if(ec) throw std::runtime_error("ディレクトリを読み込めません: " + root.string() + ": " + ec.message());
		for(; it != end; it.increment(ec)) {
			if(ec) throw std::runtime_error("ディレクトリを読み込めません: " + root.string() + ": " + ec.message());
			entries.emplace(it->path().filename(), *it);
		}
		return entries;
	}

	void compareDirectories(const fs::path &left, const fs::path &right) {
		const auto a = listDirectory(left);
		const auto b = listDirectory(right);
		auto i = a.begin();
		auto j = b.begin();
		bool found = false;
		while(i != a.end() || j != b.end()) {
			if(j == b.end() || (i != a.end() && i->first < j->first)) {
				std::cout << "左のみ: " << (left / i->first).string() << '\n';
				++i; found = true;
			} else if(i == a.end() || j->first < i->first) {
				std::cout << "右のみ: " << (right / j->first).string() << '\n';
				++j; found = true;
			} else {
				std::error_code ecA, ecB;
				const bool dirA = i->second.is_directory(ecA);
				const bool dirB = j->second.is_directory(ecB);
				if(ecA || ecB) throw std::runtime_error("項目の種類を確認できません: " + i->first.string());
				if(dirA != dirB) {
					std::cout << "種類が異なります: " << (left / i->first).string() << " | " << (right / j->first).string() << '\n';
					found = true;
				} else if(!dirA) {
					if(hashFile(left / i->first) != hashFile(right / j->first)) {
						std::cout << "相違: " << (left / i->first).string() << " | " << (right / j->first).string() << '\n';
						found = true;
					}
				}
				++i; ++j;
			}
		}
		if(!found) std::cout << "差分はありません。\n";
	}

	void usage(const char *program) {
		std::cerr << "使い方: " << program << " <ディレクトリまたはファイル> <ディレクトリまたはファイル>\n";
	}
}

int main(int argc, char *argv[]) {
#ifdef _WIN32
	// コンソールの入出力コードページを UTF-8 に統一する。
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
#endif
	if(argc != 3) {
		usage(argv[0]);
		return 2;
	}
	try {
		const fs::path left = fs::path(argv[1]);
		const fs::path right = fs::path(argv[2]);
		std::error_code ec;
		const bool existsLeft = fs::exists(left, ec);
		if(ec || !existsLeft) throw std::runtime_error("パスが存在しません: " + left.string());
		const bool existsRight = fs::exists(right, ec);
		if(ec || !existsRight) throw std::runtime_error("パスが存在しません: " + right.string());
		const bool dirLeft = isDirectory(left);
		const bool dirRight = isDirectory(right);
		if(dirLeft != dirRight) throw std::runtime_error("比較対象は同じ種類（ディレクトリ同士、またはファイル同士）を指定してください。");
		if(dirLeft) compareDirectories(left, right);
		else compareFiles(left, right);
		return 0;
	} catch(const std::exception &e) {
		std::cerr << "エラー: " << e.what() << '\n';
		return 1;
	}
}
