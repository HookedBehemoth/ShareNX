/*
 * Copyright (c) 2019 screen-nx
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "utils.hpp"

std::vector<fs::path> getDirectoryFiles(const std::string & dir, const std::vector<std::string> & extensions) {
    std::vector<fs::path> files;
    for(auto & p: fs::recursive_directory_iterator(dir))
    {
        if (fs::is_regular_file(p))
        {
                if (extensions.empty() || std::find(extensions.begin(), extensions.end(), p.path().extension().string()) != extensions.end())
            {
                files.push_back(p.path());
            }
        }
    }
    std::sort(files.begin(), files.end());
    std::reverse(files.begin(), files.end());
    return files;
}

std::string getAlbumPath() {
    std::string out = "Nintendo/Album";
    static struct
    {
        char storage_path[0x7F + 1];
        char nintendo_path[0x7F + 1];
    } __attribute__((aligned(0x1000))) paths;

    emummc_config_t config;

    int x = smcGetEmummcConfig(EMUMMC_MMC_NAND, &config, &paths);
    if(x != 0) return out;
    if(config.base_cfg.type == 0) return out;
    out = paths.nintendo_path;
    out += "/Album";
    return out;
}

Result smcGetEmummcConfig(emummc_mmc_t mmc_id, emummc_config_t *out_cfg, void *out_paths) {
    SecmonArgs args;
    args.X[0] = 0xF0000404;
    args.X[1] = mmc_id;
    args.X[2] = (u64)out_paths;
    Result rc = svcCallSecureMonitor(&args);
    if (rc == 0)
    {
        if (args.X[0] != 0)
        {
            rc = (26u | ((u32)args.X[0] << 9u));
        }
        if (rc == 0)
        {
            memcpy(out_cfg, &args.X[1], sizeof(*out_cfg));
        }
    }
    return rc;
}    
// https://github.com/AtlasNX/Kosmos-Updater/blob/master/source/FileManager.cpp#L34
bool writeFile(std::string filename, std::string data) {
    FILE * file = fopen(filename.c_str(), "w");
    if (!file) {
        return false;
    }
    size_t result = fwrite(data.c_str(), sizeof(char), data.size(), file);
    fflush(file);
    fclose(file);
    return (result == data.size());
}
// https://github.com/AtlasNX/Kosmos-Updater/blob/master/source/FileManager.cpp#L58
bool fileExists(std::string filename) {
    FILE * file = fopen(filename.c_str(), "r");
    if (file) {
        fflush(file);
        fclose(file);
        return true;
    }
    return false;
}

std::string getUrl(fs::path path) {
    std::string txtloc = path.string() + ".txt";
    std::string url;
    if(fileExists(txtloc)) {
	std::time_t currentTime = std::time(nullptr);
	std::time_t lastWriteTimePlusAWeek = std::chrono::system_clock::to_time_t(std::filesystem::last_write_time(txtloc)) + 604800;
	if (currentTime < lastWriteTimePlusAWeek) { //if txt file is younger than a week, don't upload and read from txt file
	    FILE * file = fopen(txtloc.c_str(), "r");
	    char line[1024];
	    fgets(line, 1024, file);
	    url = line;
	    fflush(file);
	    fclose(file);
	    return url;
	}
    }
    url = uploadFile(path.string());
    if(url.compare("")) writeFile(txtloc, url);//fwrite(url.c_str(), sizeof(char), url.size(), file);
    return url;
}

std::string uploadFile(std::string filePath) {
	CURL *easy = curl_easy_init();
	curl_mime *mime;
	curl_mimepart *filestuff;
	curl_mimepart *curlstuff;

	mime = curl_mime_init(easy);
	filestuff = curl_mime_addpart(mime);
	curl_mime_filedata(filestuff, filePath.c_str());
	curl_mime_name(filestuff, "fileToUpload");
	curlstuff = curl_mime_addpart(mime);
	curl_mime_data(curlstuff, "1", CURL_ZERO_TERMINATED);
	curl_mime_name(curlstuff, "curl");

	auto * urlresponse = new std::string;
	curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(easy, CURLOPT_WRITEDATA, (void *) urlresponse);
	curl_easy_setopt(easy, CURLOPT_CUSTOMREQUEST, "POST");
	curl_easy_setopt(easy, CURLOPT_MIMEPOST, mime);
	curl_easy_setopt(easy, CURLOPT_URL, "https://lewd.pics/p/index.php");
    curl_easy_setopt(easy, CURLOPT_SSL_VERIFYPEER, 0L);

	CURLcode res = curl_easy_perform(easy);
    if (res != CURLE_OK) {
        printf("FUCK!\n");
    }

	int rcode;
	curl_easy_getinfo(easy, CURLINFO_RESPONSE_CODE, &rcode);

	printf("\nresponse code: %d\n", rcode);
	printf("\nurl: %s\n", urlresponse->c_str());
	curl_easy_cleanup(easy);
	curl_mime_free(mime);

    return *urlresponse;
}

size_t WriteCallback(const char *contents, size_t size, size_t nmemb, std::string *userp) {
	userp->append(contents, size * nmemb);
	return size * nmemb;
}
