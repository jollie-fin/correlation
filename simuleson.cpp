#include <vector>
#include <map>
#include <tuple>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <sndfile.hh>
#include <string>
#include <cmath>
#include <algorithm>
#include <utility>


const double celerite = 340.;
int samplerate = 0;

std::vector < std::tuple <double, double, double> > coor_micros;
std::map < std::string, std::pair < std::vector <double> , std::vector < std::tuple <double, double, double> > > > sources_audio;
std::vector < double > sortie;
std::string nom_sortie;

void print_data()
{
	std::cout << "Micros:" << std::endl;
	double total_calcul = 0.;
	
	for (auto &i : coor_micros)
	{
		double x,y,z;
		std::tie(x,y,z) = i;
		std::cout << "(" << x << "," << y << "," << z << ")" << std::endl;
	}

	std::cout << std::endl;
	std::cout << "Sources:" << std::endl;
	for (auto &i : sources_audio)
	{
		double x,y,z;
		
		const auto &v = i.second.first;
		const auto &s = i.first;
		std::cout << s << ":" << v.size() << " ech lus" << std::endl;
		
		for (auto &j : i.second.second)
		{
			std::tie(x,y,z) = j;
			std::cout << "(" << x << "," << y << "," << z << ")" << std::endl;
		}
		total_calcul += i.second.first.size() * i.second.second.size();
	}
	
	total_calcul *= coor_micros.size();
	
	std::cout << std::endl;
	std::cout << "taille sortie : " << sortie.size() << std::endl;
	std::cout << "samplerate : " << samplerate << std::endl;
	std::cout << "total calcul : " << total_calcul << std::endl;
}

void input_data(std::string &str)
{
	std::ifstream in(str);

	int nombre_coor_micros;
	
	{
		in >> nombre_coor_micros;
		for (int i = 0; i < nombre_coor_micros; ++i)
		{
			double x,y,z;
			in >> x >> y >> z;
			coor_micros.push_back(std::make_tuple (x,y,z));
		}
	}
	
	{
		int nombre_audio;
		in >> nombre_audio;
		sf_count_t longueur_sortie = 0;	
		
		for (int i = 0; i < nombre_audio; i++)
		{
			std::string s;
			in >> s;


			SndfileHandle snd(s);

			std::vector <double> vdata;
			if (snd.channels() == 1 && (samplerate == 0 || samplerate == snd.samplerate()))
			{
				samplerate = snd.samplerate();
				vdata.resize(snd.frames());
				snd.read(&(vdata[0]), snd.frames());
				longueur_sortie = std::max(longueur_sortie, snd.frames());
			}
			else
			{
				std::cerr << "Fichier " << s << " a deux canaux ou n'a pas le même samplerate" << std::endl;
			}

			int nombre_coor_sources;
			in >> nombre_coor_sources;
			std::vector < std::tuple < double, double, double > > vcoor;
						
			for (int j = 0; j < nombre_coor_sources; ++j)
			{
				double x,y,z;
				in >> x >> y >> z;
				vcoor.push_back(std::make_tuple (x,y,z));
			}
			
			if (sources_audio.find(s) == sources_audio.end())
			{
				sources_audio[s] = std::make_pair(vdata, vcoor);
			}
		}
		sortie.resize(longueur_sortie);
	}
	in >> nom_sortie;
}

double calcule_distance(double x1, double y1, double z1, double x2, double y2, double z2)
{
	return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2)+(z1-z2)*(z1-z2));
}

double sinc(double in)
{
	if (in == 0.)
		return 1.;
	else
		return sin(in)/in;
}

void ajoute_contribution(const std::tuple <double, double, double> &micro,
							const std::tuple <double, double, double> &source,
							const std::vector <double> &onde)
{
	const int longueur_filtre = 11;
	double xm, ym, zm;
	std::tie(xm, ym, zm) = micro;
	double xs, ys, zs;
	std::tie(xs, ys, zs) = source;
	
	double distance = calcule_distance(xm,ym,zm,xs,ys,zs);
	double decalage = -distance/celerite*samplerate;
	double fdecalage;
	double integerpartofdecalage;
	fdecalage = std::modf(decalage, &integerpartofdecalage);
	int idecalage = integerpartofdecalage-.5;
	
	//http://www.labbookpages.co.uk/audio/beamforming/fractionalDelay.html
	int centre_filtre = longueur_filtre / 2;  // Position of centre FIR tap

	double coefficients[longueur_filtre];
	for (int i = 0; i < longueur_filtre; i++)
	{
		// Calculated shifted x position
		double x = i - fdecalage;

		// Calculate sinc function value
		double sc = sinc(M_PI * (x - centre_filtre));

		// Calculate (Hamming) windowing function value
		double window = 0.54 - 0.46 * std::cos(2.0 * M_PI * (x+0.5) / longueur_filtre);

		// Calculate tap weight
		coefficients[i] = window * sc / distance;
	}

	
	for (size_t i = 0; i < std::min(onde.size()-longueur_filtre, sortie.size()+idecalage); i++)
	{
		double val = 0.;
		for (int j = 0; j < longueur_filtre; j++)
		{
			val += onde[i+j] * coefficients[j];
		}
		sortie[i-idecalage] += val;
	}
}

void save_data()
{
	double maxi = 0.;
	for (auto i : sortie)
	{
		maxi = std::max(maxi, std::abs(i));
	}
	std::cout << "valeur_maximale : " << maxi << std::endl;

	for (auto &i : sortie)
	{
		i /= maxi;
	}

	
	SndfileHandle file (nom_sortie, SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_16, 1, samplerate);
	file.write (&(sortie[0]), sortie.size());	
}

int main(int argc, char **argv)
{
	
	if (argc != 2)
	return 1;
	std::string s(argv[1]); 
	
	input_data(s);
	print_data();
	for (auto &micro : coor_micros)
		for (auto &source : sources_audio)
			for (auto &coor_source : source.second.second)
				ajoute_contribution(micro, coor_source, source.second.first);
			
	save_data();
	return 0;
}
