#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stdlib.h>

#define INITIAL_CAPACITY 1000
#define LOAD_FACTOR 0.75

typedef struct {
    int tipo_dia;
    char *cod_variante;
    char *frecuencia;
    char *cod_ubic_parada;
    char *ordinal;
    char *hora;
    char *dia_anterior;
    char *X;
    char *Y;
} VFT;

typedef struct {
    char *id;
    char *codigoEmpresa;
    char *frecuencia;
    char *codigoBus;
    char *variante;
    char *linea;
    char *sublinea;
    char *tipoLinea;
    char *destino;
    char *subsistema;
    char *version;
    char *velocidad;
    char *latitud;
    char *longitud;
    char *fecha;
} VFD;

typedef struct Entry {
    char *key;
    VFT **vfts;
    VFD **vfds;
    size_t vft_count;
    size_t vft_capacity;
    size_t vfd_count;
    size_t vfd_capacity;
    struct Entry *next;
} Entry;

typedef struct {
    Entry **buckets;
    size_t size;
    size_t count;
} HashMap;

HashMap *create_hash_map();
void free_hash_map(HashMap *map);
int hash_map_insert_vft(HashMap *map, const char *key, VFT *vft);
Entry *hash_map_insert_vfd(HashMap *map, const char *key, VFD *vfd);
Entry *hash_map_search(HashMap *map, const char *key);
void resize_hash_map(HashMap *map);
char **get_all_keys(HashMap *map, size_t *key_count);
void print_hash_map(HashMap *map);
void free_vft(VFT *vft);
void free_vfd(VFD *vfd);
VFT* create_vft();
VFD* create_vfd();
void repoint_vfts_to_vfd_map(Entry* vfd_entry, Entry *vft_entry);
Entry * insert_to_vfds(Entry *entry, VFD *vfd);

#endif // HASH_MAP_H
