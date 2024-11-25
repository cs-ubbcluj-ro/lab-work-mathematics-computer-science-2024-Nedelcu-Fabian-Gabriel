#include <iostream>
#include <fstream>

#include <string>
#include <vector>

typedef std::basic_string<wchar_t> wstring;
#include <windows.h>
#include <codecvt>
#include <locale>

#include <future>

struct State
{
	bool final = false;
	std::string name;
	std::vector<std::vector<bool>> transitions;
};

class Automata
{
public:
	bool valid(std::string string)
	{
		for (int i = 0; i < string.length(); i++)
			string[i] = positionOfLetter(string[i]);
		return validFromState(0, string);
	}
	Automata(std::ifstream &fin)
	{
		statesSize = alphabetSize = 0;
		int numberStates, numberLetters;
		fin >> numberStates >> numberLetters;

		alphabet = std::vector<char>(numberStates, 0);
		states = std::vector<State>(numberStates);
		for (int i = 0; i < numberStates; i++)
		{
			states[i].transitions = std::vector<std::vector<bool>>(numberStates);
			for (int j = 0; j < numberStates; j++)
				states[i].transitions[j] = std::vector<bool>(numberLetters, false);
		}

		std::string stateName;

		std::string line;
		int first;
		int position;
		while (!fin.eof())
		{
			std::getline(fin, line);

			first = line.find_first_of(':');
			if (first <= 0)
				continue;
			stateName = line.substr(0, first);
			int statePosition = positionOfStateAdd(stateName);
			line = line.substr(first + 2);

			while (!line.empty())
			{
				first = line.find_first_of(',');
				std::string transitions;
				if (first <= 0)
				{
					transitions = line.substr(0, std::string::npos);
					line = "";
				}
				else
				{
					transitions = line.substr(0, first);
					line = line.substr(first + 2);
				}

				if (stateName == "FinalStates")
					states[positionOfStateAdd(transitions.substr(0, first))].final = true;
				else
				{
					first = transitions.find_first_of(' ');
					std::string targetState = transitions.substr(0, first);
					int targetPosition = positionOfStateAdd(targetState);
					transitions = transitions.substr(first + 1);

					do
					{
						first = transitions.find_first_of(' ');
						int usingLetter;
						if (first <= 0)
						{
							usingLetter = positionOfLetterAdd(transitions[0]);
							transitions = "";
						}
						else
						{
							usingLetter = positionOfLetterAdd(transitions.substr(0, first)[0]);
							transitions = transitions.substr(first + 1);
						}
						states[statePosition].transitions[targetPosition][usingLetter] = true;
					} while (!transitions.empty());
				}
			}
		}
	}
	std::wstring Q()
	{
		std::wstring q = L"Q = {";
		std::wstring separator = L", ";
		for (int i = 0; i < statesSize; i++)
			q += toWstring(states[i].name) + separator;
		q = q.substr(0, q.size() - 1);
		q[q.size() - 1] = L'}';
		return q;
	}
	std::wstring Σ()
	{
		std::wstring e = L"Σ = {";
		std::wstring separator = L", ";
		for (int i = 0; i < alphabetSize; i++)
			e += (wchar_t)alphabet[i] + separator;
		e = e.substr(0, e.size() - 1);
		e[e.size() - 1] = L'}';
		return e;
	}
	std::wstring F()
	{
		std::wstring f = L"F = {";
		std::wstring separator = L", ";
		for (int i = 0; i < statesSize; i++)
			if (states[i].final)
				f += toWstring(states[i].name) + separator;
		f = f.substr(0, f.size() - 1);
		f[f.size() - 1] = L'}';
		return f;
	}
	std::wstring δ()
	{
		std::wstring delta = L"\n";//L"δ : Q X Σ -> Q,\n";
		std::wstring separator = L"\n";
		for (int i = 0; i < statesSize; i++)
			for (int j = 0; j < statesSize; j++)
				for (int k = 0; k < alphabetSize; k++)
					if (states[i].transitions[j][k])
						delta += L"δ(" + toWstring(states[i].name) + L", " + (wchar_t)alphabet[k] + L") = " + toWstring(states[j].name) + separator;
		return delta;
	}
	std::wstring toString()
	{  
		return Q() + L"\n" + Σ() + L"\n" + F() + L"\n" + δ();
	}

private:
	std::wstring toWstring(const std::string& stringToConvert)
	{
		return wstring(stringToConvert.begin(), stringToConvert.end());
	}
	int positionOfStateAdd(std::string name)
	{
		int i;
		for (i = 0; i < statesSize; i++)
		{
			if (states[i].name == name)
				return i;
		}
		if (i < states.size())
		{
			states[i].name = name;
			statesSize = i + 1;
			return i;
		}
		return -1;
	}
	int positionOfLetter(char letter)
	{
		int i;
		for (i = 0; i < alphabetSize; i++)
			if (alphabet[i] == letter)
				return i;
		return -1;
	}
	int positionOfLetterAdd(char letter)
	{
		int i;
		for (i = 0; i < alphabetSize; i++)
			if (alphabet[i] == letter)
				return i;
		if (i < alphabet.size())
		{
			alphabet[i] = letter;
			alphabetSize = i + 1;
			return i;
		}
		return -1;
	}

private:
	bool validFromState(int state, std::string string)
	{
		if (string.empty())
			return states[state].final;
		int letter = string[0]; string = string.substr(1);
		std::vector<int> vector;
		for (int j = 0; j < statesSize; j++)
			if (states[state].transitions[j][letter])
				if (validFromState(j, string))
					return true;
		return false;
		/*if (string.empty())
			return states[state].final;
		int letter = string[0]; string = string.substr(1);
		std::vector<int> vector;
		for (int j = 0; j < statesSize; j++)
			if (states[state].transitions[j][letter])
					vector.push_back(j);
		for (int i = 0; i < vector.size(); i++)
			if (validFromState(vector[i], string))
				return true;
		return false;*/
	}
	int statesSize;
	int alphabetSize;
	std::vector<State> states;
	std::vector<char> alphabet;
};

int main()
{
	std::ifstream automataFile("automata.in");
	Automata automata(automataFile);

	/*for (int i = 0; i < automata.states.size(); i++)
	{
		std::cout << automata.states[i].name << ": ";
		for (int j = 0; j < automata.states[i].transitions.size(); j++)
			for (int k = 0; k < automata.states[i].transitions[j].size(); k++)
				if (automata.states[i].transitions[j][k] == true)
					std::cout << automata.states[j].name << ' ' << automata.alphabet[k] << ' ';
		if (automata.states[i].final)
			std::cout << "Final!";
		std::cout << '\n';
	}
	std::cout << "\n\n\n\n";*/

	SetConsoleOutputCP(CP_UTF8);
	setlocale(LC_ALL, "");
	std::wcout << automata.toString();

	std::ifstream wordsFile("words.in");
	std::string line;
	while (std::getline(wordsFile, line))
	{
		std::wcout << wstring(line.begin(), line.end());
		if (automata.valid(line))
			std::wcout << L" --valid";
		std::wcout << L'\n';
	}
}