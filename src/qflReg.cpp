﻿#include <algorithm>
#include <fstream>
#include <forward_list>
#include <array>
#include <vector>
#include <functional>      // For greater<int>( )
#include <iostream>
#include <assert.h>
#include "QflReg.h"
#include <string>

#ifdef WIN32
#include <windows.h>
#include <sys/utime.h>
#include <sstream>
#include <ctime>
#else
#include <ctime>
#endif

using namespace std;

static bool QDbyListSize(std::pair<std::string, std::vector<int32_t>> const & a, std::pair<std::string, std::vector<int32_t>> const & b)
{
	return a.second.size() != b.second.size() ? a.second.size() > b.second.size() : a.first.size() > b.first.size();
};



QflReg::QflReg()
{
	m_data.clear();
	m_registrants.clear();
}

QflReg::~QflReg()
{}

std::string QflReg::intToString(int i)
{
	std::stringstream ss;
	std::string s;
	ss << i;
	s = ss.str();

	return s;
}

std::string QflReg::getCurTime()
{
	std::string cur_time;
#ifdef WIN32
	SYSTEMTIME  system_time;
	GetLocalTime(&system_time);
	cur_time = std::to_string(system_time.wYear) + "_" +
		std::to_string(system_time.wMonth) + "_" +
		std::to_string(system_time.wDay);
#else
	std::stringstream currentDateTime;
	// current date/time based on current system
	time_t ttNow = time(0);
	struct tm * now = localtime(&ttNow);
	cur_time = std::to_string(now->tm_year + 1900) + std::to_string(now->tm_mon + 1) + std::to_string(now->tm_mday);
#endif
	return cur_time;
}

int32_t QflReg::readChurchList(const char* churchname)
{
	int status;
	status = m_church_list.readInChurchList(churchname);
	m_church_list.sortbyState();
	return status;
}


int32_t QflReg::readRegistrants(const char *filename)
{
	int32_t i = 0;
	if (filename == NULL)
		return -1;

	std::ifstream fin(filename);
	if (!fin.is_open())
		return -2;

	std::vector<std::string> tmp;
	size_t y;
	// read in all the data
	while (!fin.eof()) {
		std::string line0, line1;
		size_t pos = std::string::npos;
		std::getline(fin, line0);
		pos = line0.find_last_of("\"");
		while (pos != line0.size() - 1) {
			line1.clear();
			std::getline(fin, line1);
			line0 = line0 + line1;
			pos = line0.find_last_of("\"");
		}

		tmp.clear();
		//loop through the line,
		while ((pos = line0.find("\",")) != std::string::npos)
		{
			//extract the component sans ,
			pos++;
			tmp.push_back(line0.substr(0, pos));
			//erase the val including ,
			line0.erase(0, pos + 1);
		}
		tmp.push_back(line0);
		line0.erase();
		m_data.push_back(tmp);
		i++;
	}

	std::vector<std::string> person = m_data[0];
	size_t n = person.size();

	for (y = 1; y < m_data.size(); y++){
		person = m_data[y];

		if (person.size() != n) {
			m_data.erase(m_data.begin() + y);
			y--;
		}
	}

	return 0;
}


int32_t QflReg::parseAllFields()
{
	int32_t i;
	Registrant a_regist;
	if (m_data.size() == 0)
		return -1;

	std::vector<std::string> person = m_data[0];
	for (i = 1; i < m_data.size(); i++){
		person = m_data[i];
		if (person[0].size() > 2)
			a_regist.person_id = std::stoi(person[0].substr(1, person[0].size() - 2));
		if (person[1].size() > 2)
			a_regist.party = std::stoi(person[1].substr(1, person[1].size() - 2));
		if (person[2].size() > 2)
			a_regist.church = person[2].substr(1, person[2].size() - 2);
		if (person[3].size() > 2)
			a_regist.contact_person = person[3].substr(1, person[3].size() - 2);
		a_regist.party_type = person[4].substr(1, person[4].size() - 2);
		a_regist.first_name = person[5].substr(1, person[5].size() - 2);
		a_regist.last_name = person[6].substr(1, person[6].size() - 2);
		a_regist.chinese_name = person[7].substr(1, person[7].size() - 2);

		a_regist.room = person[8].substr(1, person[8].size() - 2);
		a_regist.cell_group = person[9].substr(1, person[9].size() - 2);
		a_regist.age_group = person[11].substr(1, person[11].size() - 2);
		a_regist.gender = person[12].substr(1, person[12].size() - 2);
		a_regist.grade = person[13].substr(1, person[13].size() - 2);

		a_regist.need_ride = person[20].substr(1, person[20].size() - 2);
		a_regist.offer_ride = person[21].substr(1, person[21].size() - 2);
		a_regist.is_christian = (person[22].substr(1, person[22].size() - 2).compare("Yes") == 0);
		a_regist.occupation = person[23].substr(1, person[23].size() - 2);
		a_regist.mobile_phone = person[24].substr(1, person[24].size() - 2);
		a_regist.email = person[25].substr(1, person[25].size() - 2);
		a_regist.city = person[26].substr(1, person[26].size() - 2);;
		a_regist.state = person[27].substr(1, person[27].size() - 2);;
		a_regist.zip = std::stoi(person[28].substr(1, person[28].size() - 2));
		a_regist.functional_group = person[29].substr(1, person[29].size() - 2);
		a_regist.services = person[30].substr(1, person[30].size() - 2);
		a_regist.need_room = (person[10].substr(1, person[10].size() - 2).compare(NeedRoom::RoomNeeded) == 0);
		m_registrants.push_back(a_regist);
	}


	return 0;
}

int32_t QflReg::classifications()
{
	int32_t i, j;
	if (m_registrants.size() == 0)
		return -1;

	Registrant a_regist, b_regist;
	Family a_family;
	for (i = 0; i < m_registrants.size(); i++){
		a_regist = m_registrants[i];
		if (a_regist.grade.find("Stay with Youth") != std::string::npos)
			m_youth_camp_list.push_back(a_regist.person_id);
		if (a_regist.grade.find("Stay with Parent") != std::string::npos)
			m_youth_stays_with_parent_list.push_back(a_regist.person_id);
		if (a_regist.services.find("Service Youth") != std::string::npos || a_regist.cell_group.find("Youth SGLeaders") != std::string::npos)
			m_youth_leader_list.push_back(a_regist.person_id);
		if (a_regist.services.find("Service Child") != std::string::npos)
			m_child_leader_list.push_back(a_regist.person_id);
		if (a_regist.is_christian)
			m_christian_list.push_back(a_regist.person_id);
		else
			m_non_christian_list.push_back(a_regist.person_id);

		if (a_regist.services.find("Logistics") != std::string::npos || a_regist.services.find("Recording") != std::string::npos ||
			a_regist.services.find("Traffic Control") != std::string::npos || a_regist.services.find("Usher") != std::string::npos ||
			a_regist.services.find("Friday Snack Service") != std::string::npos || a_regist.services.find("Recording") != std::string::npos)
			m_logistics_list.push_back(a_regist.person_id);

		// family registrations
		if (a_regist.party_type.find(PartyType::qFamily) != std::string::npos) {
			a_family.party = a_regist.party;
			a_family.church = a_regist.church;
			a_family.city = a_regist.city;
			a_family.contact_person = a_regist.contact_person;
			a_family.email = a_regist.email;
			a_family.mobile_phone = a_regist.mobile_phone;
			a_family.state = a_regist.state;
			a_family.zip = a_regist.zip;
			bool matched = false;
			std::map<int32_t, Family>::iterator it = m_family_list.begin();
			while (it != m_family_list.end()) {
				int32_t party_index = it->first;
				Family family = it->second;
				if (party_index == a_regist.party && family.contact_person.compare(a_regist.contact_person) == 0) {
					matched = true;
					m_family_list[a_family.party].attendee_list.push_back(a_regist.person_id);
				}
				it++;
			}
			if (!matched) {
				m_family_list[a_family.party] = a_family;
				m_family_list[a_family.party].attendee_list.push_back(i);
			}
		}
		// individual registrations
		else {
			if (a_regist.gender.compare("Female") == 0) {
				m_female_list.push_back(a_regist.person_id);
			}
			else {
				m_male_list.push_back(a_regist.person_id);
			}
		}
	}


	return 0;
}

int32_t QflReg::sortAttendeesByChurches()
{
	int32_t i, j;
	std::vector<int32_t> attendee_list;
	if (m_registrants.size() == 0)
		return -1;

	std::vector<ChurchList::QFLChurch> *church_list = m_church_list.getChurchList();
	m_attendee_list_byChurch.resize(church_list->size());
	for (j = 0; j < church_list->size(); j++) {
		m_attendee_list_byChurch[j].first = (*church_list)[j].church_name;
		m_attendee_list_byChurch[j].second = attendee_list;
	}

	Registrant a_regist;
	for (i = 0; i < m_registrants.size(); i++){
		a_regist = m_registrants[i];
		for (j = 0; j < church_list->size(); j++) {
			std::string church_name = (*church_list)[j].church_name;
			std::string church_ini = (*church_list)[j].church_ini;
			int32_t church_code = (*church_list)[j].church_code;

			if (a_regist.church.compare(church_name) == 0) {
				m_attendee_list_byChurch[j].second.push_back(a_regist.person_id);
				break;
			}
		}
	}

	std::sort(m_attendee_list_byChurch.begin(), m_attendee_list_byChurch.end(), QDbyListSize);
	return 0;
}

int32_t QflReg::ageStatistics()
{
	int32_t i, j;
	std::vector<int32_t> attendee_list;
	if (m_registrants.size() == 0)
		return -1;

	m_age_statistics.resize(13);
	for (i = 0; i < m_age_statistics.size(); i++)
		m_age_statistics[i] = 0;

	Registrant a_regist;
	for (i = 0; i < m_registrants.size(); i++){
		a_regist = m_registrants[i];
		if (a_regist.age_group.find(AgeGroup::A70) != std::string::npos)
			m_age_statistics[12]++;
		else if (a_regist.age_group.find(AgeGroup::A66_69) != std::string::npos)
			m_age_statistics[11]++;
		else if (a_regist.age_group.find(AgeGroup::A56_65) != std::string::npos)
			m_age_statistics[10]++;
		else if (a_regist.age_group.find(AgeGroup::A40_55) != std::string::npos)
			m_age_statistics[9]++;
		else if (a_regist.age_group.find(AgeGroup::A26_39) != std::string::npos)
			m_age_statistics[8]++;
		else if (a_regist.age_group.find(AgeGroup::A18_25) != std::string::npos)
			m_age_statistics[7]++;
		else if (a_regist.age_group.find(AgeGroup::A15_17) != std::string::npos)
			m_age_statistics[6]++;
		else if (a_regist.age_group.find(AgeGroup::A12_14) != std::string::npos)
			m_age_statistics[5]++;
		else if (a_regist.age_group.find(AgeGroup::A6_11) != std::string::npos)
			m_age_statistics[4]++;
		else if (a_regist.age_group.find(AgeGroup::A4_5) != std::string::npos)
			m_age_statistics[3]++;
		else if (a_regist.age_group.find(AgeGroup::A3) != std::string::npos)
			m_age_statistics[2]++;
		else if (a_regist.age_group.find(AgeGroup::A2) != std::string::npos)
			m_age_statistics[1]++;
		else if (a_regist.age_group.find(AgeGroup::A1) != std::string::npos)
			m_age_statistics[0]++;
		else
			printf("Invalid age\n");
	}

	return 0;
}

void QflReg::printOutStatistics(const char*filename)
{
	int32_t i;
	if (filename == NULL)
		return;
	FILE *hf = fopen(filename, "w+");
	if (hf == NULL)
		return;

	// print number of registrants
	fprintf(hf, "Time = %s\n", getCurTime().c_str());
	fprintf(hf, "Registrants: %d\n", m_registrants.size());
	fprintf(hf, "Christians = %d, non-christian = %d\n", m_christian_list.size(), m_non_christian_list.size());

	// print top 15 church list
	for (i = 0; i < 15; i++) {
		fprintf(hf, "%s, %d\n", m_attendee_list_byChurch[i].first.c_str(), m_attendee_list_byChurch[i].second.size());
	}

	// print family, male, female
	fprintf(hf, "family: %d, male: %d, female: %d\n", m_family_list.size(), m_male_list.size(), m_female_list.size());

	// print youth with parents
	fprintf(hf, "Youth with Parents: %d\n", m_youth_stays_with_parent_list.size());
	// print youth in camp
	fprintf(hf, "Youth in Camp: %d\n", m_youth_camp_list.size());
	// print youth leaders
	fprintf(hf, "Youth leader: %d\n", m_youth_leader_list.size());
	// print Child care
	fprintf(hf, "Child care: %d\n", m_child_leader_list.size());

	// print age distribution
	fprintf(hf, "%s, %d\n", AgeGroup::A1, m_age_statistics[0]);
	fprintf(hf, "%s, %d\n", AgeGroup::A2, m_age_statistics[1]);
	fprintf(hf, "%s, %d\n", AgeGroup::A3, m_age_statistics[2]);
	fprintf(hf, "%s, %d\n", AgeGroup::A4_5, m_age_statistics[3]);
	fprintf(hf, "%s, %d\n", AgeGroup::A6_11, m_age_statistics[4]);
	fprintf(hf, "%s, %d\n", AgeGroup::A12_14, m_age_statistics[5]);
	fprintf(hf, "%s, %d\n", AgeGroup::A15_17, m_age_statistics[6]);
	fprintf(hf, "%s, %d\n", AgeGroup::A18_25, m_age_statistics[7]);
	fprintf(hf, "%s, %d\n", AgeGroup::A26_39, m_age_statistics[8]);
	fprintf(hf, "%s, %d\n", AgeGroup::A40_55, m_age_statistics[9]);
	fprintf(hf, "%s, %d\n", AgeGroup::A56_65, m_age_statistics[10]);
	fprintf(hf, "%s, %d\n", AgeGroup::A66_69, m_age_statistics[11]);
	fprintf(hf, "%s, %d\n", AgeGroup::A70, m_age_statistics[12]);

	fclose(hf);

	return;
}

void QflReg::printOutForChildWorkers(const char*filename)
{
	int32_t i, j;
	if (filename == NULL)
		return;
	FILE *hf = fopen(filename, "w+");
	if (hf == NULL)
		return;

	fprintf(hf, "Person ID, Chinese Name, First Name, Last Name, Gender, Age, Contact, ");
	fprintf(hf, "Service, Church, City, State, Function Group, Email\n");

	for (i = 0; i < m_child_leader_list.size(); i++) {
		int32_t p = m_child_leader_list[i];

		for (j = 0; j < m_registrants.size(); j++) {
			if (p == m_registrants[j].person_id) {
				Registrant person = m_registrants[j];
				fprintf(hf, "%d, %s, %s, %s, %s, %s, %s, ", person.person_id, person.chinese_name.c_str(), person.first_name.c_str(), person.last_name.c_str(), person.gender.c_str(), person.age_group.c_str(), person.contact_person.c_str());
				fprintf(hf, "%s, %s, %s, %s, %s, %s\n", person.services.c_str(), person.church.c_str(), person.city.c_str(), person.state.c_str(), person.functional_group.c_str(), person.email.c_str());
			}
		}
	}
	fclose(hf);
	return;
}


void QflReg::printOutForLogistics(const char*filename)
{
	int32_t i, j;
	if (filename == NULL)
		return;
	FILE *hf = fopen(filename, "w+");
	if (hf == NULL)
		return;

	for (i = 0; i < m_logistics_list.size(); i++) {
		int32_t p = m_logistics_list[i];

		for (j = 0; j < m_registrants.size(); j++) {
			if (p == m_registrants[j].person_id) {
				Registrant person = m_registrants[j];
				fprintf(hf, "%d, %s, %s, %s, %s, %s, ", person.person_id, person.chinese_name.c_str(), person.first_name.c_str(), person.last_name.c_str(), person.gender.c_str(), person.age_group.c_str());
				fprintf(hf, "%s, %s, %s, %s, %s, %s\n", person.services.c_str(), person.church.c_str(), person.city.c_str(), person.state.c_str(), person.functional_group.c_str(), person.email.c_str());
			}
		}
	}
	fclose(hf);
	return;
}

void QflReg::printOutForYouth(const char*filename)
{
	int32_t i, j;
	if (filename == NULL)
		return;
	FILE *hf = fopen(filename, "w+");
	if (hf == NULL)
		return;

	fprintf(hf, "Youth Campers\n");
	for (i = 0; i < m_youth_camp_list.size(); i++) {
		int32_t p = m_youth_camp_list[i];

		for (j = 0; j < m_registrants.size(); j++) {
			if (p == m_registrants[j].person_id) {
				Registrant person = m_registrants[j];
				fprintf(hf, "%d, %s, %s, %s, %s, %s, ", person.person_id, person.chinese_name.c_str(), person.first_name.c_str(), person.last_name.c_str(), person.gender.c_str(), person.age_group.c_str());
				fprintf(hf, "%s, %s, %s, %s, %s, %s\n", person.services.c_str(), person.church.c_str(), person.city.c_str(), person.state.c_str(), person.functional_group.c_str(), person.email.c_str());
			}
		}
	}

	fprintf(hf, "Youth Camper Leaders\n");
	for (i = 0; i < m_youth_leader_list.size(); i++) {
		int32_t p = m_youth_leader_list[i];

		for (j = 0; j < m_registrants.size(); j++) {
			if (p == m_registrants[j].person_id) {
				Registrant person = m_registrants[j];
				fprintf(hf, "%d, %s, %s, %s, %s, %s, ", person.person_id, person.chinese_name.c_str(), person.first_name.c_str(), person.last_name.c_str(), person.gender.c_str(), person.age_group.c_str());
				fprintf(hf, "%s, %s, %s, %s, %s, %s\n", person.services.c_str(), person.church.c_str(), person.city.c_str(), person.state.c_str(), person.functional_group.c_str(), person.email.c_str());
			}
		}
	}

	fprintf(hf, "Youth Stay with Parents\n");
	for (i = 0; i < m_youth_stays_with_parent_list.size(); i++) {
		int32_t p = m_youth_stays_with_parent_list[i];

		for (j = 0; j < m_registrants.size(); j++) {
			if (p == m_registrants[j].person_id) {
				Registrant person = m_registrants[j];
				fprintf(hf, "%d, %s, %s, %s, %s, %s, ", person.person_id, person.chinese_name.c_str(), person.first_name.c_str(), person.last_name.c_str(), person.gender.c_str(), person.age_group.c_str());
				fprintf(hf, "%s, %s, %s, %s, %s, %s\n", person.services.c_str(), person.church.c_str(), person.city.c_str(), person.state.c_str(), person.functional_group.c_str(), person.email.c_str());
			}
		}
	}

	fclose(hf);
	return;
}

void QflReg::printRCCCFunctions(const char *dirname)
{
	int32_t i, j;
	if (dirname == NULL)
		return;

	// xiang yin
	std::string xiangyin_name = std::string(dirname) + "/" + "xiangyin.csv";
	FILE *hf = fopen(xiangyin_name.c_str(), "w+");
	if (hf == NULL)
		return;

	fprintf(hf, "Person ID, Chinese Name, First Name, Last Name, Gender, Age, Contact, ");
	fprintf(hf, "Service, City, State, Function Group, Email\n");

	const std::string RCCC = "Rutgers Community Christian Church";
	// XiangYin
	for (j = 0; j < m_registrants.size(); j++) {
		if (m_registrants[j].church.compare(RCCC) == 0) {
			Registrant person = m_registrants[j];
			std::string fg = person.functional_group;

			if (fg.find(std::string("æ„›ä¹‹å…‰")) != std::string::npos || fg.find("ä¹¡éŸ³") != std::string::npos || 
				fg.find("é„‰éŸ³") != std::string::npos || fg.find("Xiang Yin") != std::string::npos ||
				fg.find("ç¤¾é’") != std::string::npos)
			{
				fprintf(hf, "%d, %s, %s, %s, %s, %s, %s, ", person.person_id, person.chinese_name.c_str(), person.first_name.c_str(), person.last_name.c_str(), person.gender.c_str(), person.age_group.c_str(), person.contact_person.c_str());
				fprintf(hf, "%s, %s, %s, %s, %s\n", person.services.c_str(), person.city.c_str(), person.state.c_str(), person.functional_group.c_str(), person.email.c_str());
			}
		}
	}
	fclose(hf);

	// choir
	std::string choir_name = std::string(dirname) + "/" + "choir.csv";
	hf = fopen(choir_name.c_str(), "w+");
	if (hf == NULL)
		return;

	for (j = 0; j < m_registrants.size(); j++) {
		if (m_registrants[j].church.compare(RCCC) == 0) {
			Registrant person = m_registrants[j];
			std::string chor = person.functional_group;

			if (chor.find(std::string("è¯—ç­")) != std::string::npos || chor.find("è©©ç") != std::string::npos || chor.find(std::string("Choir")) != std::string::npos)
			{
				fprintf(hf, "%d, %s, %s, %s, %s, %s, %s, ", person.person_id, person.chinese_name.c_str(), person.first_name.c_str(), person.last_name.c_str(), person.gender.c_str(), person.age_group.c_str(), person.contact_person.c_str());
				fprintf(hf, "%s, %s, %s, %s, %s\n", person.services.c_str(), person.city.c_str(), person.state.c_str(), person.functional_group.c_str(), person.email.c_str());
			}
		}
	}
	fclose(hf);

	// student fellowship
	std::string sfellowship_name = std::string(dirname) + "/" + "student_fellowship.csv";
	hf = fopen(sfellowship_name.c_str(), "w+");
	if (hf == NULL)
		return;

	for (j = 0; j < m_registrants.size(); j++) {
		if (m_registrants[j].church.compare(RCCC) == 0) {
			Registrant person = m_registrants[j];
			std::string sf = person.functional_group;

			if (sf.find(std::string("­å­¦ç”Ÿåœ˜å¥‘")) != std::string::npos || sf.find("å­¦ç”Ÿå›¢å¥‘") != std::string::npos)
			{
				fprintf(hf, "%d, %s, %s, %s, %s, %s, %s, ", person.person_id, person.chinese_name.c_str(), person.first_name.c_str(), person.last_name.c_str(), person.gender.c_str(), person.age_group.c_str(), person.contact_person.c_str());
				fprintf(hf, "%s, %s, %s, %s, %s\n", person.services.c_str(), person.city.c_str(), person.state.c_str(), person.functional_group.c_str(), person.email.c_str());
			}
		}
	}
	fclose(hf);

	return;
}

void QflReg::printRCCCAttendees(const char*filename)
{
	int32_t i, j;
	if (filename == NULL)
		return;
	FILE *hf = fopen(filename, "w+");
	if (hf == NULL)
		return;

	fprintf(hf, "Person ID, Chinese Name, First Name, Last Name, Gender, Age, Contact, ");
	fprintf(hf, "Service, City, State, Function Group, Email\n");

	const std::string RCCC = "Rutgers Community Christian Church";
	for (j = 0; j < m_registrants.size(); j++) {
		if (m_registrants[j].church.compare(RCCC) == 0) {
			Registrant person = m_registrants[j];
			fprintf(hf, "%d, %s, %s, %s, %s, %s, %s, ", person.person_id, person.chinese_name.c_str(), person.first_name.c_str(), person.last_name.c_str(), person.gender.c_str(), person.age_group.c_str(), person.contact_person.c_str());
			fprintf(hf, "%s, %s, %s, %s, %s\n", person.services.c_str(), person.city.c_str(), person.state.c_str(), person.functional_group.c_str(), person.email.c_str());
		}
	}

	fclose(hf);
	return;
}

void QflReg::printLivingStone(const char*filename)
{
	int32_t i, j;
	if (filename == NULL)
		return;
	FILE *hf = fopen(filename, "w+");
	if (hf == NULL)
		return;

	fprintf(hf, "Person ID, Chinese Name, First Name, Last Name, Gender, Age, Contact, ");
	fprintf(hf, "Service, City, State, Function Group, Email\n");

	const std::string LSCC = "Living Stone Christian Church";
	for (j = 0; j < m_registrants.size(); j++) {
		if (m_registrants[j].church.compare(LSCC) == 0) {
			Registrant person = m_registrants[j];
			fprintf(hf, "%d, %s, %s, %s, %s, %s, %s, ", person.person_id, person.chinese_name.c_str(), person.first_name.c_str(), person.last_name.c_str(), person.gender.c_str(), person.age_group.c_str(), person.contact_person.c_str());
			fprintf(hf, "%s, %s, %s, %s, %s\n", person.services.c_str(), person.city.c_str(), person.state.c_str(), person.functional_group.c_str(), person.email.c_str());
		}
	}

	fclose(hf);
	return;
}

void QflReg::printCherryHill(const char*filename)
{
	int32_t i, j;
	if (filename == NULL)
		return;
	FILE *hf = fopen(filename, "w+");
	if (hf == NULL)
		return;

	fprintf(hf, "Person ID, Chinese Name, First Name, Last Name, Gender, Age, Contact, ");
	fprintf(hf, "Service, City, State, Function Group, Email\n");

	const std::string CHCCC = "Cherry Hill Chinese Christian Church";
	for (j = 0; j < m_registrants.size(); j++) {
		if (m_registrants[j].church.compare(CHCCC) == 0) {
			Registrant person = m_registrants[j];
			fprintf(hf, "%d, %s, %s, %s, %s, %s, %s, ", person.person_id, person.chinese_name.c_str(), person.first_name.c_str(), person.last_name.c_str(), person.gender.c_str(), person.age_group.c_str(), person.contact_person.c_str());
			fprintf(hf, "%s, %s, %s, %s, %s\n", person.services.c_str(), person.city.c_str(), person.state.c_str(), person.functional_group.c_str(), person.email.c_str());
		}
	}

	fclose(hf);
	return;
}

void QflReg::printPhillyTrinity(const char*filename)
{
	int32_t i, j;
	if (filename == NULL)
		return;
	FILE *hf = fopen(filename, "w+");
	if (hf == NULL)
		return;

	fprintf(hf, "Person ID, Chinese Name, First Name, Last Name, Gender, Age, Contact, ");
	fprintf(hf, "Service, City, State, Function Group, Email\n");

	const std::string TCCGP = "Trinity Christian Church of Greater Philadelphia";
	for (j = 0; j < m_registrants.size(); j++) {
		if (m_registrants[j].church.compare(TCCGP) == 0) {
			Registrant person = m_registrants[j];
			fprintf(hf, "%d, %s, %s, %s, %s, %s, %s, ", person.person_id, person.chinese_name.c_str(), person.first_name.c_str(), person.last_name.c_str(), person.gender.c_str(), person.age_group.c_str(), person.contact_person.c_str());
			fprintf(hf, "%s, %s, %s, %s, %s\n", person.services.c_str(), person.city.c_str(), person.state.c_str(), person.functional_group.c_str(), person.email.c_str());
		}
	}

	fclose(hf);
	return;
}

void QflReg::printPhillyCCC(const char*filename)
{
	int32_t i, j;
	if (filename == NULL)
		return;
	FILE *hf = fopen(filename, "w+");
	if (hf == NULL)
		return;

	fprintf(hf, "Person ID, Chinese Name, First Name, Last Name, Gender, Age, Contact, ");
	fprintf(hf, "Service, City, State, Function Group, Email\n");

	const std::string CCCCNC = "Chinese Christian Church & Center";
	for (j = 0; j < m_registrants.size(); j++) {
		if (m_registrants[j].church.compare(CCCCNC) == 0) {
			Registrant person = m_registrants[j];
			fprintf(hf, "%d, %s, %s, %s, %s, %s, %s, ", person.person_id, person.chinese_name.c_str(), person.first_name.c_str(), person.last_name.c_str(), person.gender.c_str(), person.age_group.c_str(), person.contact_person.c_str());
			fprintf(hf, "%s, %s, %s, %s, %s\n", person.services.c_str(), person.city.c_str(), person.state.c_str(), person.functional_group.c_str(), person.email.c_str());
		}
	}

	fclose(hf);
	return;

}



void QflReg::printDoubtfulRegistrants(const char*filename)
{
	int32_t i, j;
	if (filename == NULL)
		return;
	FILE *hf = fopen(filename, "w+");
	if (hf == NULL)
		return;

	for (j = 0; j < m_registrants.size(); j++) {
		Registrant p = m_registrants[j];
		std::string first = p.first_name;
		std::string last = p.last_name;
		std::string age = p.age_group;
		std::string grade = p.grade;
		std::string occupation = p.occupation;

		for (i = j + 1; i < m_registrants.size(); i++) {
			Registrant person = m_registrants[i];
			if (person.first_name.compare(first) == 0 && person.last_name.compare(last) == 0) {
				fprintf(hf, "%d, %s, %s, %s, %s, %s, %s, ", p.person_id, p.party_type.c_str(), p.chinese_name.c_str(), p.first_name.c_str(), p.last_name.c_str(), p.gender.c_str(), p.age_group.c_str());
				fprintf(hf, "%s, %s, %s, %s, %s, %s, %s\n", p.services.c_str(), p.contact_person.c_str(), p.church.c_str(), p.city.c_str(), p.state.c_str(), p.functional_group.c_str(), p.email.c_str());
				fprintf(hf, "%d, %s, %s, %s, %s, %s, %s, ", person.person_id, person.party_type.c_str(), person.chinese_name.c_str(), person.first_name.c_str(), person.last_name.c_str(), person.gender.c_str(), person.age_group.c_str());
				fprintf(hf, "%s, %s, %s, %s, %s, %s, %s\n", person.services.c_str(), person.contact_person.c_str(), person.church.c_str(), person.city.c_str(), person.state.c_str(), person.functional_group.c_str(), person.email.c_str());
			}
		}
		if (occupation.compare("Minister") == 0) {
			fprintf(hf, "%d, %s, %s, %s, %s, %s, %s, ", p.person_id, p.party_type.c_str(), p.chinese_name.c_str(), p.first_name.c_str(), p.last_name.c_str(), p.gender.c_str(), p.age_group.c_str());
			fprintf(hf, "%s, %s, %s, %s, %s, %s, %s, %s\n", p.services.c_str(), p.contact_person.c_str(), p.church.c_str(), p.city.c_str(), p.state.c_str(), p.functional_group.c_str(), p.email.c_str(), p.occupation.c_str());
		}
	}

	fclose(hf);
	return;
}

void QflReg::printRidesList(const char*filename)
{
	int32_t i, j;
	std::string need;
	std::string offer;
	int32_t need_num, offer_num;

	for (j = 0; j < m_registrants.size(); j++) {
		Registrant p = m_registrants[j];
		int32_t id = p.party;
		std::string need_ride = p.need_ride;
		std::string offer_ride = p.offer_ride;
		need_num = 0;
		offer_num = 0;

		if (!need_ride.empty()) {
			need = need_ride.substr(0, 1);
			need_num = std::atoi(need.c_str());
		}

		if (!offer_ride.empty()) {
			offer = offer_ride.substr(0, 1);
			offer_num = std::atoi(offer.c_str());
		}
		if (offer_num > 0) {
			bool matched = false;
			for (i = 0; i < m_ride_list.size(); i++) {
				if (m_ride_list[i] == id) {
					matched = true;
					break;
				}
			}
			if (!matched)
				m_ride_list.push_back(id);
		}
		else if (need_num > 0) {
			bool matched = false;
			for (i = 0; i < m_need_ride_list.size(); i++) {
				if (m_need_ride_list[i] == id) {
					matched = true;
					break;
				}
			}
			if (!matched)
				m_need_ride_list.push_back(id);
		}
	}

	if (filename == NULL)
		return;
	FILE *hf = fopen(filename, "w+");
	if (hf == NULL)
		return;

	fprintf(hf, "Party ID, party Type, Chinese Name, First Name, Last Name, Gender, Age, ");
	fprintf(hf, "Need Ride, Offer Ride, Contact Person, Church, City, State, Function Group, Email\n");

	for (i = 0; i < m_ride_list.size(); i++) {
		int32_t id = m_ride_list[i];
		for (j = 0; j < m_registrants.size(); j++) {
			Registrant p = m_registrants[j];
			int32_t pid = p.party;
			if (id == pid) {
				fprintf(hf, "%d, %s, %s, %s, %s, %s, %s, ", p.party, p.party_type.c_str(), p.chinese_name.c_str(), p.first_name.c_str(), p.last_name.c_str(), p.gender.c_str(), p.age_group.c_str());
				fprintf(hf, ", %s, %s, %s, %s, %s, %s, %s\n", p.offer_ride.substr(0, 1).c_str(), p.contact_person.c_str(), p.church.c_str(), p.city.c_str(), p.state.c_str(), p.functional_group.c_str(), p.email.c_str());
				break;
			}
		}
	}

	for (i = 0; i < m_need_ride_list.size(); i++) {
		int32_t id = m_need_ride_list[i];
		for (j = 0; j < m_registrants.size(); j++) {
			Registrant p = m_registrants[j];
			int32_t pid = p.party;
			if (id == pid) {
				fprintf(hf, "%d, %s, %s, %s, %s, %s, %s, ", p.party, p.party_type.c_str(), p.chinese_name.c_str(), p.first_name.c_str(), p.last_name.c_str(), p.gender.c_str(), p.age_group.c_str());
				fprintf(hf, "%s, , %s, %s, %s, %s, %s, %s\n", p.need_ride.substr(0, 1).c_str(), p.contact_person.c_str(), p.church.c_str(), p.city.c_str(), p.state.c_str(), p.functional_group.c_str(), p.email.c_str());
				break;
			}
		}
	}

	fclose(hf);
	return;
}