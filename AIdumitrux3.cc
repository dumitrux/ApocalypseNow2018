#include "Player.hh"


/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME dumitrux3

struct PLAYER_NAME : public Player {

  /**
   * Factory: returns a new instance of this class.
   * Do not modify this function.
   */
  static Player* factory () {
    return new PLAYER_NAME;
  }

  /**
   * Types and attributes for your player can be defined here.
   */
	// Celdas alrededor de los soldados
	static constexpr int SI[8] = { 1, 1, 0, -1, -1, -1,  0,  1 };
	static constexpr int SJ[8] = { 0, 1, 1,  1,  0, -1, -1, -1 };
	
	// Celdas cubiertas por los helicopteros
	static constexpr int HI[5] = {-2, -1, 0, 1, 2};
	static constexpr int HJ[5] = {-2, -1, 0, 1, 2};
	
	// Movimientos permitidos para los helicopteros
	static constexpr int HI2[4] = { 1, 0, -1, 0 };
	static constexpr int HJ2[4] = { 0, 1, 0, -1 };
	
	typedef vector<int> vec;
	typedef vector<vector<int> > tablero;
	
	// Devuelve cierto si es una posicion valida para el helicopteros
	bool posicion_segura_h(int x, int y) {
		bool correcta = true;
		int i, j;
		i = j = 0;
		while (i < 5 and correcta) {
			while (j < 5 and correcta) {
				Position h;
				h.i = x + HI[i]; 
				h.j = y + HJ[j];
				correcta = what(h.i, h.j) != MOUNTAIN;
				++j;
			}
			++i;
		}
		return correcta;
	}
	
	// Devuelve si la celda es cesped o bosque, no hay fuego y no hay un soldado propio
	bool posicio_segura(Position pos, int id) {
		int tipo_suelo, fuego, mi_jugador, jugador2;
		mi_jugador = me();
		tipo_suelo = what(pos.i, pos.j);
		fuego = fire_time(pos.i, pos.j);
		jugador2 = data(id).player;
		return ((tipo_suelo == FOREST or tipo_suelo == GRASS) and fuego == 0 and 
		jugador2 != mi_jugador and jugador2 >= 0);
	}
	
	// Devuelve cierto si ataca. Ataca si hay un soldado enemigo en una celda adyacente
	bool atacar(int id) {
		Data s = data(id);
		Position pos_s;
		int k, mi_jugador;
		bool atacar = false;
		pos_s = s.pos;
		mi_jugador = s.player;
		k = 0;
		while (k < 8 and !atacar) {
			// idc, identificador de soldado en celda adyacente
			Position pos_se;
			int id_cerca;
			pos_se.i = pos_s.i + SI[k];
			pos_se.j = pos_s.j + SJ[k];
			id_cerca = which_soldier(pos_se.i, pos_se.j);
			if (id_cerca > 0 and data(id_cerca).player != mi_jugador) {
				// Si hay un enemigo alrededor se ataca
				command_soldier(id, pos_se.i, pos_se.j);
				atacar = true;
			}
			++k;
		}
		return atacar;
	}
	
	// Devuelve el numero de celdas de alrededor que sean bosque y esten ardiendo
	int fuego_alrededor(Position pos) {
		int k, fuego;
		k = fuego = 0;
		if (what(pos.i, pos.j) == FOREST) {
			while (k < 8) {
				Position pos_se;
				pos_se.i = pos.i + SI[k];
				pos_se.j = pos.j + SJ[k];
				if (fire_time(pos_se.i, pos_se.j) > 0) ++fuego;
				++k;
			}
		}
		return fuego;
	}
	
	// Devuelve cierto si hay un soldado enemigo adyacente a la celda
	bool enemigo_alrededor(Position pos) {
		int mi_jugador = me();
		int k = 0;
		while (k < 8) {
			Position pos_se;
			int id_cerca;
			pos_se.i = pos.i + SI[k];
			pos_se.j = pos.j + SJ[k];
			id_cerca = which_soldier(pos_se.i, pos_se.j);
			if (id_cerca > 0 and data(id_cerca).player != mi_jugador) return true;
			++k;
		}
		return false;
	}
	
	// bfs para encontrar camino a un post
	void play_soldier(int id) {
		Position pos = data(id).pos;
		queue<Position> Q;
		tablero busqueda(MAX, vector<int>(MAX, -1));
		Position rumbo;
		stack<Position> ruta;
		int mi_jugador = me();
		bool post_encontrado = false;
		Q.push(pos);
		busqueda[pos.i][pos.j] = 0;
		while(Q.size() > 0 and !post_encontrado) {
			Position rumbo = Q.front();
			ruta.push(rumbo);
			Q.pop();
			int post_conq = post_owner(rumbo.i, rumbo.j);
			
			if (post_conq != mi_jugador and post_conq != -2 and 
			posicio_segura(rumbo, id) == 0) {
				post_encontrado = true;
			}
			else {
				int k = 0;
				while(k < 8 and !post_encontrado) {
					Position aux;
					int tipo_suelo, fuego;
					aux.i = rumbo.i + SI[k];
					aux.j = rumbo.j + SJ[k];
					tipo_suelo = what(aux.i, aux.j);
					fuego = fire_time(aux.i, aux.j);
					if((tipo_suelo == FOREST or tipo_suelo == GRASS) and fuego == 0 and busqueda[aux.i][aux.j] == -1) {
						Q.push(aux);
						busqueda[aux.i][aux.j] = busqueda[rumbo.i][rumbo.j] + 1;
					}
					++k;
				}
			}
		}
		vector<Position> posibles;
		vector<int> valor_fuego;
		rumbo = ruta.top();// Post encontrado
		
		ruta.pop();
		Position destino2;
		while (ruta.size() > 1) {// Como llegar al post
			Position ady;
			bool adyacente, nivel;
			adyacente = false;
			int k = 0;
			ady = ruta.top();
			if(busqueda[rumbo.i][rumbo.j] == 2) destino2 = rumbo;
			nivel = (busqueda[rumbo.i][rumbo.j] == busqueda[ady.i][ady.j]);
			while (k < 8 and !adyacente and !nivel) {
				ady.i += SI[k];
				ady.j += SJ[k];
				adyacente = ((rumbo.i == ady.i) and (rumbo.j == ady.j));
				ady = ruta.top();
				++k;
			}
			// Guarda las posibles celdas que pueden ir el soldado
			// para llegar a la segunda celda de camino al post 
			if(busqueda[ady.i][ady.j] == 1) {
				while(ruta.size() > 1) {
					Position x = ruta.top();
					ruta.pop();
					posibles.push_back(x);
				}
			}
			
			ruta.pop();
			if (adyacente) rumbo = ady;
		}
		
		vector<Position> posibles2;
		for(int i2 = 0; i2 < (int)posibles.size(); ++i2) {
			Position ady2;
			bool encontrado2 = false;
			int j2 = 0;
			while(j2 < 8 and !encontrado2) {
				ady2 = posibles[i2];
				ady2.i += SI[j2];
				ady2.j += SJ[j2];
				if ((destino2.i == ady2.i) and (destino2.j == ady2.j)) {
					posibles2.push_back(posibles[i2]);
					encontrado2 = true;
				}
				++j2;
			}
		}
		
		int nn2 = posibles2.size();
		valor_fuego = vector<int>(nn2);
		for (int i = 0; i < nn2; ++i) {
			// Numero de celdas con fuego alrededor de un post
			// con el mismo indice que posibles2
			valor_fuego[i] = fuego_alrededor(posibles2[i]);
		}
		
		bool coms = false;
		int k;
		k = 0;
		int que;
		if(posibles2.empty()) {// La siguiente celda es el post
			command_soldier(id, rumbo.i, rumbo.j);
			coms = true;
		}
		
		// Con las siguientes comprobaciones se "convierte bfs a un mini-Dijkstra",
		// con los posibles caminos a la proxima celda se elije el mejor,
		// seguno enemigos, fuego alrededor y tipo de terreno
		while (k < nn2 and !coms) {
			que = what(posibles2[k].i, posibles2[k].j);
			if(que == FOREST and !enemigo_alrededor(posibles2[k]) 
			and valor_fuego[k] == 0 and which_soldier(posibles2[k].i, posibles2[k].j) == 0) {
				command_soldier(id, posibles2[k].i, posibles2[k].j);
				coms = true;
			}
			++k;
		}
		k = 0;
		while (k < nn2 and !coms) {
			que = what(posibles2[k].i, posibles2[k].j);
			if((que == FOREST or que == GRASS) and !enemigo_alrededor(posibles2[k]) 
			and valor_fuego[k] == 0 and which_soldier(posibles2[k].i, posibles2[k].j) == 0) {
				command_soldier(id, posibles2[k].i, posibles2[k].j);
				coms = true;
			}
			++k;
		}
		
		k = 0;
		while (k < nn2 and !coms) {
			que = what(posibles2[k].i, posibles2[k].j);
			if((que == FOREST or que == GRASS) and !enemigo_alrededor(posibles2[k]) 
			and valor_fuego[k] < 1 and which_soldier(posibles2[k].i, posibles2[k].j) == 0) {
				command_soldier(id, posibles2[k].i, posibles2[k].j);
				coms = true;
			}
			++k;
		}
		k = 0;
		while (k < nn2 and !coms) {
			que = what(posibles2[k].i, posibles2[k].j);
			if((que == FOREST or que == GRASS) and !enemigo_alrededor(posibles2[k]) 
			and valor_fuego[k] < 2 and which_soldier(posibles2[k].i, posibles2[k].j) == 0) {
				command_soldier(id, posibles2[k].i, posibles2[k].j);
				coms = true;
			}
			++k;
		}
		k = 0;
		while (k < nn2 and !coms) {
			que = what(posibles2[k].i, posibles2[k].j);
			if((que == FOREST or que == GRASS) and 
			!enemigo_alrededor(posibles2[k]) and valor_fuego[k] < 3 and which_soldier(posibles2[k].i, posibles2[k].j) == 0) {
				command_soldier(id, posibles2[k].i, posibles2[k].j);
				coms = true;
			}
			++k;
		}
		k = 0;
		while (k < nn2 and !coms) {
			que = what(posibles2[k].i, posibles2[k].j);
			if((que == FOREST or que == GRASS) and 
			!enemigo_alrededor(posibles2[k]) and valor_fuego[k] < 4 and which_soldier(posibles2[k].i, posibles2[k].j) == 0) {
				command_soldier(id, posibles2[k].i, posibles2[k].j);
				coms = true;
			}
			++k;
		}
		k = 0;
		while (k < nn2 and !coms) {
			que = what(posibles2[k].i, posibles2[k].j);
			if((que == FOREST or que == GRASS) and 
			!enemigo_alrededor(posibles2[k]) and valor_fuego[k] < 5 and which_soldier(posibles2[k].i, posibles2[k].j) == 0) {
				command_soldier(id, posibles2[k].i, posibles2[k].j);
				coms = true;
			}
			++k;
		}
		k = 0;
		while (k < nn2 and !coms) {
			que = what(posibles2[k].i, posibles2[k].j);
			if(posicio_segura(posibles2[k], id)) {
				command_soldier(id, posibles2[k].i, posibles2[k].j);
			}
			++k;
		}
	}
	
	// Tira un nombre dado de paracaidistas en zona segura si puede
	void throw_parachuter(int id, int para, int p) {
		Data h = data(id);
		int p_tirados = 0;
		for (int i = 0; i < 5; ++i) {
			for (int j = 0; j < 5; ++j) {
				Position pos_h;
				int s, tipo_suelo, fuego;
				pos_h.i = h.pos.i + HI[i];
				pos_h.j = h.pos.j + HJ[j];
				s = which_soldier(pos_h.i, pos_h.j);
				tipo_suelo = what(pos_h.i, pos_h.j);
				fuego = fire_time(pos_h.i, pos_h.j);
				// Hay paracaidas en el H, solo tirar 2, no hay soldado aliado y posicion segura
				if (p != 0 and para != 0 and s == 0 and p_tirados < p and (tipo_suelo == 1 or tipo_suelo == 2) 
				and fuego == 0) {
					command_parachuter(pos_h.i, pos_h.j);
					++p_tirados;
					--para;
				}
			}
		}
	}
	
	// Devuelve cierto si un helicoptero tiene un post a dos casillas o menos
	bool post_cerca(Position pos) {
		bool cerca = false;
		int i, j, mi_jugador;
		mi_jugador = me();
		i = j = 0;
		while (i < 5 and !cerca) {
			while (j < 5 and !cerca) {
				Position h;
				int post;
				h.i = pos.i + HI[i]; 
				h.j = pos.j + HJ[j];
				post = post_owner(h.i, h.j);
				cerca = ((post == -1 or post != mi_jugador) and post != -2);
				++j;
			}
			++i;
		}
		return cerca;
	}
	
	// Devuelve cierto si el helicoptero ha tirado napalm
	bool tirar_napalm(int id) {
		Data h = data(id);
		int enemigos, mis_sold, mi_jugador;
		bool napalm = false;
		enemigos = mis_sold = 0;
		mi_jugador = h.player;
		
		if (h.napalm == 0) {
			for (int i = 0; i < 5; ++i) {
				for (int j = 0; j < 5; ++j) {
					Position pos_h;
					int idcerca;
					pos_h.i = h.pos.i + HI[i];
					pos_h.j = h.pos.j + HJ[j];
					idcerca = which_soldier(pos_h.i, pos_h.j);
					if (idcerca != 0 and data(idcerca).player != mi_jugador) ++enemigos;
					if (idcerca != 0 and data(idcerca).player == mi_jugador) ++mis_sold;
				}
			}
			if (mis_sold < enemigos and enemigos > 2) {
				// Lanzar napalm
				command_helicopter(id, NAPALM);
				napalm = true;
			}
		}
		return napalm;
	}
	
	// bfs para encontrar camino a un post
	void play_helicopter(int id) {
		Data h = data(id);
		Position pos = h.pos;
		queue<Position> Q;
		tablero busqueda(MAX, vector<int>(MAX, -1));
		Position rumbo;
		stack<Position> ruta;
		bool post_c = false;
		Q.push(pos);
		busqueda[pos.i][pos.j] = 0;
		while(Q.size() > 0 and !post_c) {
			Position rumbo = Q.front();
			ruta.push(rumbo);
			Q.pop();
			if (post_cerca(rumbo)) {
				post_c = true;
			}
			else {
				int k = 0;
				while(k < 4 and !post_c) {
					Position aux;
					aux.i = rumbo.i + HI2[k];
					aux.j = rumbo.j + HJ2[k]; 
					if(pos_ok(aux) and posicion_segura_h(aux.i, aux.j) and busqueda[aux.i][aux.j] == -1) {
						Q.push(aux);
						busqueda[aux.i][aux.j] = busqueda[rumbo.i][rumbo.j] + 1;
					}
					++k;
				}
			}
		}
		rumbo = ruta.top();// Post
		ruta.pop();
		while (ruta.size() > 1) {// Recorre camino al post
			Position ady;
			bool adyacente, nivel;
			adyacente = false;
			int k = 0;
			ady = ruta.top();
			nivel = (busqueda[rumbo.i][rumbo.j] == busqueda[ady.i][ady.j]);
			while (k < 4 and !adyacente and !nivel) {
				ady.i += HI2[k];
				ady.j += HJ2[k];
				adyacente = ((rumbo.i == ady.i) and (rumbo.j == ady.j));
				ady = ruta.top();
				++k;
			}
			ruta.pop();
			if (adyacente) rumbo = ady;
		}
		
		if (post_cerca(pos)) { // Si hay post cerca se mueve alrededor
			bool movh = false;
			if (!movh and posicion_segura_h(pos.i+1, pos.j) and h.orientation == 0) {
				command_helicopter(id, FORWARD1);
				movh = true;
			}
			if (!movh and posicion_segura_h(pos.i, pos.j+1) and h.orientation == 1) {
				command_helicopter(id, FORWARD1);
				movh = true;
			}
			if (!movh and posicion_segura_h(pos.i-1, pos.j) and h.orientation == 2) {
				command_helicopter(id, FORWARD1);
				movh = true;
			}
			if (!movh and posicion_segura_h(pos.i, pos.j-1) and h.orientation == 3) {
				command_helicopter(id, FORWARD1);
				movh = true;
			}
			if(!movh) command_helicopter(id, CLOCKWISE);
		}
		
		// Segun el siguiente movimiento y la orientacion se elige el movimiento
		else if ((rumbo.i == pos.i+1 and rumbo.j == pos.j and h.orientation == 0 and posicion_segura_h(pos.i+1, pos.j)) or
		(rumbo.i == pos.i and rumbo.j == pos.j+1 and h.orientation == 1 and posicion_segura_h(pos.i, pos.j+1)) or
		(rumbo.i == pos.i-1 and rumbo.j == pos.j and h.orientation == 2 and posicion_segura_h(pos.i-1, pos.j)) or
		(rumbo.i == pos.i and rumbo.j == pos.j-1 and h.orientation == 3 and posicion_segura_h(pos.i, pos.j-1))) command_helicopter(id, FORWARD1);
		else if (rumbo.i == pos.i+1) command_helicopter(id, CLOCKWISE);
		else command_helicopter(id, COUNTER_CLOCKWISE);
	}
	

  /**
   * Play method, invoked once per each round.
   */
  virtual void play () {
	  int mi_jugador = me();
	  vec H = helicopters(mi_jugador); //Mis helicopteros
	  vec S = soldiers(mi_jugador); // Mis soldados
	  
	  // Soldados
	  for (int i = 0; i < (int)S.size(); ++i) {
		  if (!atacar(S[i])) play_soldier(S[i]);
	  }
	  
	  // Helicopteros
	  
	  // Se elige cuantos paracaidistas tirar en cada helicoptero
	  int p1, p2;
	  p1 = data(H[0]).parachuters.size();
	  p2 = data(H[1]).parachuters.size();
	  if (p1 >= 2*p2) {
		  p1 = 3;
		  p2 = 1;
	  }
	  else if (p2 >= p1*2) {
		  p2 = 3;
		  p1 = 1;
	  }
	  else {
		  p1 = 2;
		  p2 = 2;
	  }
	  for (int i = 0; i < (int)H.size(); ++i) {
		  if(!tirar_napalm(H[i])) {
			  int p, para;
			  if (i == 0) p = p1;
			  else p = p2;
			  para = data(H[i]).parachuters.size();
			  if (para != 0) throw_parachuter(H[i], para, p);
			  play_helicopter(H[i]);
			}
	  }
  }
};

constexpr int PLAYER_NAME::SI[8];
constexpr int PLAYER_NAME::SJ[8];

constexpr int PLAYER_NAME::HI[5];
constexpr int PLAYER_NAME::HJ[5];

constexpr int PLAYER_NAME::HI2[4];
constexpr int PLAYER_NAME::HJ2[4];


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);
