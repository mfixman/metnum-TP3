#include <cassert>
#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <chrono>

#include "camaralenta.h"
#include "vecinomascercano.h"
#include "interpolacionlineal.h"
#include "splines.h"

using namespace std::chrono;
using namespace std;

enum class Metodo
{
	vecinoMasCercano = 1,
	interpolacionLineal = 2,
	interpolacionPorSplines = 3
};

void usage(char *name)
{
	std::cout << "Usage: " << name << " entrada salida metodo cuadros_a_agregar [reset]" << std::endl;
	exit(1);
}

CamaraLenta *parsearEntrada(FILE *in, Metodo m, int reset)
{
	int c;
	fscanf(in, "%d\n", &c);

	int h, w;
	fscanf(in, "%d,%d\n", &h, &w);

	int f;
	fscanf(in, "%d\n", &f);

	Pelicula p(h, std::vector <PixelTiempo> (w, PixelTiempo(c)));
	for (int t = 0; t < c; t++)
	{
		for (int i = 0; i < h; i++)
		{
			for (int u = 0; u < w; u++)
			{
				int g;
				std::fscanf(in, "%d,", &g);
				p[i][u][t] = g;
			}
		}
	}

	if (reset == -1)
		reset = c;

	switch (m)
	{
		case Metodo::vecinoMasCercano:
			return new VecinoMasCercano(p, c, f);

		case Metodo::interpolacionLineal:
			return new InterpolacionLineal(p, c, f);

		case Metodo::interpolacionPorSplines:
			return new Splines(p, c, f, reset);
	}

	std::cerr << "Metodo invalido! Esto no deberia pasar." << std::endl;
	std::abort();
}

void escribirSalida(FILE *out, const Pelicula &p, int f)
{
	assert(!p.empty() && !p[0].empty());

	int c = p[0][0].size();
	int h = p.size(), w = p[0].size();

	std::fprintf(out, "%d\n", c);
	std::fprintf(out, "%d,%d\n", h, w);
	std::fprintf(out, "%d\n", f);

	for (int t = 0; t < c; t++)
	{
		for (int i = 0; i < h; i++)
		{
			for (int j = 0; j < w; j++)
				std::fprintf(out, "%d%c", p[i][j][t], j == w - 1 ? '\n' : ',');
		}
	}
}

int main(int argc, char *argv[])
{
	if (argc < 5 || argc > 6)
		usage(argv[0]);
	
	char *entrada = argv[1];
	char *salida = argv[2];

	Metodo m = Metodo(atoi(argv[3]));
	int dc = atoi(argv[4]);

	int reset = -1;
	if (argc == 6)
		reset = atoi(argv[5]);

	FILE *in = fopen(entrada, "r");
	FILE *out = fopen(salida, "w");

	if (int(m) <= 0 || int(m) > 3 || dc < 0 || in == NULL)
		usage(argv[0]);

	CamaraLenta *q = parsearEntrada(in, m, reset);
	
	Pelicula r;
	
	auto duration = 0;
	for (int i = 0; i < 10; i++)
	{
		high_resolution_clock::time_point t1 = high_resolution_clock::now();
		r = q->alentar(dc);
		high_resolution_clock::time_point t2 = high_resolution_clock::now();
		
		duration += std::chrono::duration_cast<std::chrono::milliseconds>( t2 - t1 ).count();
	}
	
	if (argc == 6)
		cout << argv[5] << ",";
	cout << argv[4] << "," << duration / 10 << endl;

	escribirSalida(out, r, q->f);
	return 0;
}
