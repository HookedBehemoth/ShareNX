/*
 * Copyright (c) 2019-2020 ShareNX
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
#include "util/host.hpp"

#include <curl/curl.h>

#include <chrono>

#include "util/caps.hpp"
#include "util/common.hpp"

static size_t MovieRead(char *buffer, size_t size, size_t nitems, void *userdata) {
	return ((caps::MovieReader *)userdata)->Read(buffer, size * nitems);
}

static size_t StringWrite(void *contents, size_t size, size_t nmemb, void *userdata) {
	((std::string *)userdata)->append((char *)contents, size * nmemb);
	return size * nmemb;
}

static size_t xferCb(void *userdata, u32 dltotal, u32 dlnow, u32 ultotal, u32 ulnow) {
	auto layout = static_cast<ui::UploadLayout *>(userdata);
	layout->setProgress(((double)ulnow / (double)ultotal) * 100.0);
	return 0;
}

Hoster::Hoster() {}

Hoster::~Hoster() {}

void Hoster::Initialize(const nlohmann::json &json, std::string name) {
	this->name = name;
	this->url = common::GetString(json, "Url", "");
	this->regex = common::GetString(json, "Regex", "");
	this->imageMimeName = common::GetString(json, "ImageName", "");
	this->videoMimeName = common::GetString(json, "VideoName", "");
	this->parse = common::GetBool(json, "Show Response", true);
	this->mimeData.clear();
	if (!json.contains("Mimepart"))
		return;
	auto &mime = json["Mimepart"];
	if (mime == nullptr || !mime.is_array())
		return;
	for (auto &elm : mime) {
		this->mimeData.push_back({common::GetString(elm, "Name", "name"),
								  common::GetString(elm, "Data", "1")});
	}
}

void Hoster::SetDefault() {
	this->name = "";
	this->url = "https://lewd.pics/p/index.php";
	this->regex = "";
	this->imageMimeName = "fileToUpload";
	this->videoMimeName = "fileToUpload";
	this->parse = true;
	this->mimeData = {{"curl", "1"}};
}

std::string Hoster::GetName() {
	if (this->name.empty()) {
		return "lewd.pics";
	}
	return this->name;
}

std::string Hoster::GetUrl() {
	return this->url;
}

std::string Hoster::GetRegex() {
	return this->regex;
}

std::string Hoster::uploadEntry(const CapsAlbumEntry &entry, ui::UploadLayout *cb_data) {
	CURL *curl;
	CURLcode res = CURLE_OK;
	std::string readBuffer;
	long http_code = 0;
	void *imageBuffer = nullptr;
	caps::MovieReader *movieReader = nullptr;

	curl = curl_easy_init();
	if (curl) {
		curl_mime *mime = curl_mime_init(curl);

		curl_mimepart *file_part = curl_mime_addpart(mime);
		std::string fileName = caps::entryToFileName(entry);
		curl_mime_filename(file_part, fileName.c_str());
		printf("filename: %s\n", fileName.c_str());
		if (entry.file_id.content == CapsAlbumFileContents_ScreenShot || entry.file_id.content == CapsAlbumFileContents_ExtraScreenShot) {
			curl_mime_name(file_part, this->imageMimeName.c_str());
			imageBuffer = malloc(entry.size);
			caps::getFile(entry, imageBuffer);
			curl_mime_data(file_part, (const char *)imageBuffer, entry.size);
		} else {
			movieReader = new caps::MovieReader(entry);
			curl_mime_name(file_part, this->videoMimeName.c_str());
			curl_mime_data_cb(file_part, movieReader->GetStreamSize(), MovieRead, nullptr, nullptr, movieReader);
		}

		for (Mime data : this->mimeData) {
			printf("adding part: [%s: %s]\n", data.name.c_str(), data.data.c_str());
			auto part = curl_mime_addpart(mime);
			curl_mime_name(part, data.name.c_str());
			curl_mime_data(part, data.data.c_str(), CURL_ZERO_TERMINATED);
		}
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, StringWrite);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		curl_easy_setopt(curl, CURLOPT_URL, this->url.c_str());
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
		curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, xferCb);
		curl_easy_setopt(curl, CURLOPT_XFERINFODATA, cb_data);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		auto start = std::chrono::steady_clock::now();
		res = curl_easy_perform(curl);
		auto end = std::chrono::steady_clock::now();
		printf("took %f ms\n", std::chrono::duration<double, std::milli>(end - start).count());
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
		curl_easy_cleanup(curl);
		curl_mime_free(mime);
	}

	if (movieReader) {
		delete movieReader;
		movieReader = nullptr;
	}

	if (imageBuffer) {
		free(imageBuffer);
		imageBuffer = nullptr;
	}

	if (res != CURLE_OK) {
		printf("perform failed with %d\n", res);
		return "curl error " + std::to_string(res);
	}

	if (http_code != 200) {
		printf("upload failed\n");
		return "failed with http code " + std::to_string(http_code);
	}

	printf("urlresponse: %s\n", readBuffer.c_str());

	return this->parse ? readBuffer : "Success!";
}
