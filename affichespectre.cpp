#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cstdlib>
#include <cmath>
#include <complex>
#include <string>
#include <random>

const std::complex<double> I{0, 1};
std::vector<double> abscisse_micros;

double celerite = 340.;
int W, H;
double lowfreq, highfreq;
char mode;
int nombre_micros;
double largeur;

std::vector <double> value;
std::vector <char> image;

void input_data(std::string &s)
{
	std::ifstream in(s);
	in >> W >> H;
	in >> lowfreq >> highfreq;
	in >> nombre_micros;
	in >> mode;
	in >> largeur;

	abscisse_micros.resize(nombre_micros);
	std::random_device rd;
	std::mt19937 gen(rd());
    std::uniform_real_distribution<double> distribution(0.0,largeur);
	for (size_t i = 0; i < abscisse_micros.size(); i++)
	{
		if (mode=='E')
		{
			abscisse_micros[i]=largeur*double(i)/double(nombre_micros);
		}
		else
		{
			abscisse_micros[i]=distribution(gen);
		}
			
	}
}

double coefficient(double freq, double angle)
{
	double xincident = std::sin(angle);
	double xaccu = 0.;
	double yaccu = 0.;
	
	for (auto i : abscisse_micros)
	{
		double pscal = i*xincident;
		double dephasaget = pscal/celerite;
		double pulsation = dephasaget*freq*2*M_PI;
		xaccu += cos(pulsation);
		yaccu += sin(pulsation);
	}
	return sqrt(xaccu*xaccu+yaccu*yaccu)/abscisse_micros.size();
}

bool seuil(double valeur1, double valeur2, double nombre_increments)
{
	return floor(valeur1 * nombre_increments) == ceil(valeur2 * nombre_increments) && valeur1 != valeur2;
}

void compute()
{
	value.resize(H*W);
	for (int i = 0; i < H; i++)
		for (int j = 0; j < W; j++)
		{
			double x = (double(j)-double(W)/2.)/double(W)*M_PI;
			double log = std::log10(highfreq/lowfreq);
			double y = double(H-i)/double(H)*log;
			double freq = pow(10., y) * lowfreq;
			double c = coefficient(freq, x);
			value[i*W+j] = c;
		}
	image.resize(H*W*3);
	for (int i = 0; i < H; i++)
		for (int j = 0; j < W; j++)
		{
			if (value[i*W+j] < .5)
			{
					image[i*W*3+j*3+0] = 0;
					image[i*W*3+j*3+1] = 0;
					image[i*W*3+j*3+2] = 0;
			}
			else
			{
					image[i*W*3+j*3+0] = 255;
					image[i*W*3+j*3+1] = 255;
					image[i*W*3+j*3+2] = 255;
			}/*
			if (i > 1 && i < H - 1 && j > 1 && j < W - 1)
			{
				if (   seuil(value[i*W+j], value[i*W+j+1], 10)
					|| seuil(value[i*W+j], value[i*W+j-1], 10)
					|| seuil(value[i*W+j], value[i*W+j+W], 10)					
					|| seuil(value[i*W+j], value[i*W+j-W], 10)
				    || seuil(value[i*W+j], value[i*W+j+1+W], 10)
					|| seuil(value[i*W+j], value[i*W+j-1+W], 10)
					|| seuil(value[i*W+j], value[i*W+j+1-W], 10)					
					|| seuil(value[i*W+j], value[i*W+j-1-W], 10)
					)
				{
					image[i*W*3+j*3+0] = 0;
					image[i*W*3+j*3+1] = 0;
					image[i*W*3+j*3+2] = 0;
				}
				else
				{
					int vi = value[i*W+j] * 255.;
					image[i*W*3+j*3+0] = 255;
					image[i*W*3+j*3+1] = vi;
					image[i*W*3+j*3+2] = vi;
				}
			}
			else
			{
				int vi = value[i*W+j] * 255.;
				image[i*W*3+j*3+0] = 255;
				image[i*W*3+j*3+1] = vi;
				image[i*W*3+j*3+2] = vi;
			}*/
	
		}
}

void output()
{
	std::ostringstream ostrstr;
	ostrstr << W << "x" << H << "-" << lowfreq << "_" << highfreq << "-" << mode << "_" << nombre_micros << "_" << largeur << "m.ppm";
	std::cout << ostrstr.str();
	
	std::ofstream out(ostrstr.str(), std::ios::out | std::ios::binary);
	out << "P6" << std::endl;
	out << W << " " << H << std::endl;
	out << "255" << std::endl;
	
	out.write(&(image[0]),3*H*W);
			
//	std::cout << std::endl;
			
}

int main(int argc, char **argv)
{
	if (argc != 2)
		return 1;
	std::string s(argv[1]);
	input_data(s);
	compute();
	output();
	return 0;
}

