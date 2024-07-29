#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash_map.h"

unsigned long hash(const char *str, size_t size) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash % size;
}

HashMap *create_hash_map() {
    HashMap *map = (HashMap *)malloc(sizeof(HashMap));
    map->buckets = (Entry **)calloc(INITIAL_CAPACITY, sizeof(Entry *));
    map->size = INITIAL_CAPACITY;
    map->count = 0;

    char** campos_capturas = (char**)malloc(2 * sizeof(char*));
    campos_capturas[0] = "id";
    campos_capturas[1] = "codigoEmpresa";
    campos_capturas[2] = "frecuencia";
    campos_capturas[3] = "codigoBus";
    campos_capturas[4] = "variante";
    campos_capturas[5] = "linea";
    campos_capturas[6] = "sublinea";
    campos_capturas[7] = "tipoLinea";
    campos_capturas[8] = "destino";
    campos_capturas[9] = "subsistema";
    campos_capturas[10] = "version";
    campos_capturas[11] = "velocidad";
    campos_capturas[12] = "latitud";
    campos_capturas[13] = "longitud";
    campos_capturas[14] = "fecha";
    campos_capturas[15] = NULL; 
    map->campos_capturas = campos_capturas;

    char** campos_horarios = (char**)malloc(2 * sizeof(char*));
    campos_horarios[0] = "tipo_dia";
    campos_horarios[1] = "cod_variante";
    campos_horarios[2] = "frecuencia";
    campos_horarios[3] = "cod_ubic_parada";
    campos_horarios[4] = "ordinal";
    campos_horarios[5] = "hora";
    campos_horarios[6] = "dia_anterior";
    campos_horarios[7] = "X";
    campos_horarios[8] = "Y";
    campos_capturas[9] = NULL; 
    map->campos_horarios = campos_horarios;

    return map;
}

void free_vft(VFT *vft) {
    if (vft) {
        if (vft->cod_variante) free(vft->cod_variante);
        if (vft->frecuencia) free(vft->frecuencia);
        if (vft->cod_ubic_parada) free(vft->cod_ubic_parada);
        if (vft->ordinal) free(vft->ordinal);
        if (vft->hora) free(vft->hora);
        if (vft->dia_anterior) free(vft->dia_anterior);
        if (vft->X) free(vft->X);
        if (vft->Y) free(vft->Y);
        free(vft);
    }
}

void free_vfts(VFT **vfts, size_t count) {
    for (size_t i = 0; i < count; i++) {
        free_vft(vfts[i]);
    }
    free(vfts);
}

void free_vfd(VFD *vfd) {
    if (vfd) {
        if (vfd->id) free(vfd->id);
        if (vfd->codigoEmpresa) free(vfd->codigoEmpresa);
        if (vfd->frecuencia) free(vfd->frecuencia);
        if (vfd->codigoBus) free(vfd->codigoBus);
        if (vfd->variante) free(vfd->variante);
        if (vfd->linea) free(vfd->linea);
        if (vfd->sublinea) free(vfd->sublinea);
        if (vfd->tipoLinea) free(vfd->tipoLinea);
        if (vfd->destino) free(vfd->destino);
        if (vfd->subsistema) free(vfd->subsistema);
        if (vfd->version) free(vfd->version);
        if (vfd->velocidad) free(vfd->velocidad);
        if (vfd->latitud) free(vfd->latitud);
        if (vfd->longitud) free(vfd->longitud);
        if (vfd->fecha) free(vfd->fecha);
        free(vfd);
    }
}

void free_vfds(VFD **vfds, size_t count) {
    for (size_t i = 0; i < count; i++) {
        free_vfd(vfds[i]);
    }
    free(vfds);
}

void free_entry(Entry *entry) {
    free(entry->key);
    free_vfts(entry->vfts, entry->vft_count);
    free_vfds(entry->vfds, entry->vfd_count);
    free(entry);
}

void free_hash_map(HashMap *map) {
    for (size_t i = 0; i < map->size; i++) {
        Entry *entry = map->buckets[i];
        while (entry != NULL) {
            Entry *temp = entry;
            entry = entry->next;
            free_entry(temp);
        }
    }
    free(map->buckets);
    free(map);
}

int insert_to_vfts(Entry *entry, VFT *vft) {
    if (entry->vft_count >= entry->vft_capacity) {
        entry->vft_capacity *= 2;
        entry->vfts = (VFT **)realloc(entry->vfts, entry->vft_capacity * sizeof(VFT *));
        if (entry->vfts == NULL) {
            return -1; // Allocation failed
        }
    }
    entry->vfts[entry->vft_count] = vft;
    entry->vft_count++;
    return (int) entry->vft_count;
}

Entry * insert_to_vfds(Entry *entry, VFD *vfd) {
    if (entry->vfd_count >= entry->vfd_capacity) {
        entry->vfd_capacity *= 2;
        entry->vfds = (VFD **)realloc(entry->vfds, entry->vfd_capacity * sizeof(VFD *));
        if (entry->vfds == NULL) {
            return NULL; // Allocation failed
        }
    }
    entry->vfds[entry->vfd_count] = vfd;
    entry->vfd_count++;
    return entry;
}

void resize_hash_map(HashMap *map) {
    size_t new_size = map->size * 2;
    Entry **new_buckets = (Entry **)calloc(new_size, sizeof(Entry *));
    if (new_buckets == NULL) {
        perror("Failed to allocate memory for resized hash map");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < map->size; i++) {
        Entry *entry = map->buckets[i];
        while (entry != NULL) {
            Entry *next = entry->next;
            unsigned long new_index = hash(entry->key, new_size);
            entry->next = new_buckets[new_index];
            new_buckets[new_index] = entry;
            entry = next;
        }
    }

    free(map->buckets);
    map->buckets = new_buckets;
    map->size = new_size;
}

Entry *find_or_create_entry(HashMap *map, const char *key) {
    if ((float)map->count / map->size > LOAD_FACTOR) {
        resize_hash_map(map);
    }

    unsigned long index = hash(key, map->size);
    Entry *entry = map->buckets[index];
    Entry *prev = NULL;

    while (entry != NULL && strcmp(entry->key, key) != 0) {
        prev = entry;
        entry = entry->next;
    }

    if (entry == NULL) {
        entry = (Entry *)malloc(sizeof(Entry));
        if (entry == NULL) {
            perror("Failed to allocate memory for Entry");
            exit(EXIT_FAILURE);
        }
        entry->key = strdup(key);
        if (entry->key == NULL) {
            perror("Failed to allocate memory for key");
            exit(EXIT_FAILURE);
        }
        entry->vfts = (VFT **)malloc(2 * sizeof(VFT *));
        entry->vft_count = 0;
        entry->vft_capacity = 2;
        entry->vfds = (VFD **)malloc(2 * sizeof(VFD *));
        entry->vfd_count = 0;
        entry->vfd_capacity = 2;
        entry->next = NULL;

        if (prev == NULL) {
            map->buckets[index] = entry;
        } else {
            prev->next = entry;
        }
        map->count++;
    }
    return entry;
}

int hash_map_insert_vft(HashMap *map, const char *key, VFT *vft) {
    Entry *entry = find_or_create_entry(map, key);
    return insert_to_vfts(entry, vft);
}

Entry* hash_map_insert_vfd(HashMap *map, const char *key, VFD *vfd) {
    Entry *entry = find_or_create_entry(map, key);
    return insert_to_vfds(entry, vfd);
}

Entry *hash_map_search(HashMap *map, const char *key) {
    if (!map || !key) return NULL;

    // Compute the hash value for the key
    unsigned long index = hash(key, map->size);

    // Traverse the linked list at the corresponding bucket
    Entry *entry = map->buckets[index];
    while (entry != NULL && strcmp(entry->key, key) != 0) {
        entry = entry->next;
    }

    return entry;
}

char** get_all_keys(HashMap *map, size_t *key_count) {
    *key_count = 0;
    size_t capacity = 10;
    char **keys = malloc(capacity * sizeof(char*));

    for (size_t i = 0; i < map->size; i++) {
        Entry *entry = map->buckets[i];
        while (entry != NULL) {
            if (*key_count == capacity) {
                capacity *= 2;
                keys = realloc(keys, capacity * sizeof(char*));
            }
            keys[*key_count] = strdup(entry->key);
            (*key_count)++;
            entry = entry->next;
        }
    }

    // Sort the keys alphabetically
    qsort(keys, *key_count, sizeof(char*), (int (*)(const void*, const void*)) strcmp);

    return keys;
}

void print_hash_map(HashMap *map) {
    for (size_t i = 0; i < map->size; i++) {
        Entry *entry = map->buckets[i];
        while (entry != NULL) {
            printf("Key: %s\n", entry->key);

            printf("VFTs:\n");
            for (size_t j = 0; j < entry->vft_count; j++) {
                VFT *vft = entry->vfts[j];
                printf("  Tipo Dia: %d, Variante: %s, Frecuencia: %s, Cod Ubic Parada: %s, Ordinal: %s, Hora: %s, Dia Anterior: %s, Latitud: %s, Longitud: %s\n",
                       vft->tipo_dia, vft->cod_variante, vft->frecuencia, vft->cod_ubic_parada, vft->ordinal, vft->hora, vft->dia_anterior, vft->X, vft->Y);
            }

            printf("VFDs:\n");
            for (size_t j = 0; j < entry->vfd_count; j++) {
                VFD *vfd = entry->vfds[j];
                printf("  ID: %s, Codigo Empresa: %s, Frecuencia: %s, Codigo Bus: %s, Variante: %s, Linea: %s, Sublinea: %s, Tipo Linea: %s, Destino: %s, Subsistema: %s, Version: %s, Velocidad: %s, Latitud: %s, Longitud: %s, Fecha: %s\n",
                       vfd->id, vfd->codigoEmpresa, vfd->frecuencia, vfd->codigoBus, vfd->variante, vfd->linea, vfd->sublinea, vfd->tipoLinea, vfd->destino, vfd->subsistema, vfd->version, vfd->velocidad, vfd->latitud, vfd->longitud, vfd->fecha);
            }

            entry = entry->next;
        }
    }
}

VFT* create_vft() {
    VFT *vft = (VFT *)malloc(sizeof(VFT));
    if (vft) {
        vft->tipo_dia = 0;
        vft->cod_variante = NULL;
        vft->frecuencia = NULL;
        vft->cod_ubic_parada = NULL;
        vft->ordinal = NULL;
        vft->hora = NULL;
        vft->dia_anterior = NULL;
        vft->X = NULL;
        vft->Y = NULL;
    }
    return vft;
}

VFD* create_vfd() {
    VFD *vfd = (VFD *)malloc(sizeof(VFD));
    if (vfd) {
        vfd->id = NULL;
        vfd->codigoEmpresa = NULL;
        vfd->frecuencia = NULL;
        vfd->codigoBus = NULL;
        vfd->variante = NULL;
        vfd->linea = NULL;
        vfd->sublinea = NULL;
        vfd->tipoLinea = NULL;
        vfd->destino = NULL;
        vfd->subsistema = NULL;
        vfd->version = NULL;
        vfd->velocidad = NULL;
        vfd->latitud = NULL;
        vfd->longitud = NULL;
        vfd->fecha = NULL;
    }
    return vfd;
}

void repoint_vfts_to_vfd_map(Entry* vfd_entry, Entry *vft_entry) {
    // Repoint the vfd entry's vfts to the new entry's vfts
    vfd_entry->vfts = vft_entry->vfts;
    vfd_entry->vft_count = vft_entry->vft_count;
    vfd_entry->vft_capacity = vft_entry->vft_capacity;

    // Nullify the vft entry's vfts to avoid double free
    vft_entry->vfts = NULL;
    vft_entry->vft_count = 0;
    vft_entry->vft_capacity = 0;
}

char** get_campos_capturas(HashMap* map){
    return map->campos_capturas;
}

char** get_campos_horarios(HashMap* map){
    return map->campos_horarios;
}

VFT** get_capturas(Entry* entry){
    return entry->vfds;
}

VFD** get_horarios(Entry* entry){
    return entry->vfts;
}