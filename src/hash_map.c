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
    return map;
}

void free_vfts(VFT **vfts, size_t count) {
    for (size_t i = 0; i < count; i++) {
        free(vfts[i]->tipo_dia);
        free(vfts[i]->variante);
        free(vfts[i]->frecuencia);
        free(vfts[i]->cod_ubic_parada);
        free(vfts[i]->ordinal);
        free(vfts[i]->hora);
        free(vfts[i]->dia_anterior);
        free(vfts[i]->latitud);
        free(vfts[i]->longitud);
        free(vfts[i]);
    }
    free(vfts);
}

void free_vfds(VFD **vfds, size_t count) {
    for (size_t i = 0; i < count; i++) {
        free(vfds[i]->id);
        free(vfds[i]->codigoEmpresa);
        free(vfds[i]->frecuencia);
        free(vfds[i]->codigoBus);
        free(vfds[i]->variante);
        free(vfds[i]->linea);
        free(vfds[i]->sublinea);
        free(vfds[i]->tipoLinea);
        free(vfds[i]->destino);
        free(vfds[i]->subsistema);
        free(vfds[i]->version);
        free(vfds[i]->velocidad);
        free(vfds[i]->latitud);
        free(vfds[i]->longitud);
        free(vfds[i]->fecha);
        free(vfds[i]);
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
    return 0;
}

int insert_to_vfds(Entry *entry, VFD *vfd) {
    if (entry->vfd_count >= entry->vfd_capacity) {
        entry->vfd_capacity *= 2;
        entry->vfds = (VFD **)realloc(entry->vfds, entry->vfd_capacity * sizeof(VFD *));
        if (entry->vfds == NULL) {
            return -1; // Allocation failed
        }
    }
    entry->vfds[entry->vfd_count] = vfd;
    entry->vfd_count++;
    return 0;
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

int hash_map_insert_vfd(HashMap *map, const char *key, VFD *vfd) {
    Entry *entry = find_or_create_entry(map, key);
    return insert_to_vfds(entry, vfd);
}

Entry *hash_map_search(HashMap *map, const char *key) {
    unsigned long index = hash(key, map->size);
    Entry *entry = map->buckets[index];

    while (entry != NULL && strcmp(entry->key, key) != 0) {
        entry = entry->next;
    }

    return entry;
}

char **get_all_keys(HashMap *map, size_t *key_count) {
    *key_count = 0;
    char **keys = (char **)malloc(map->count * sizeof(char *));
    if (keys == NULL) {
        perror("Failed to allocate memory for keys array");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < map->size; i++) {
        Entry *entry = map->buckets[i];
        while (entry != NULL) {
            keys[*key_count] = strdup(entry->key);
            if (keys[*key_count] == NULL) {
                perror("Failed to allocate memory for key");
                exit(EXIT_FAILURE);
            }
            (*key_count)++;
            entry = entry->next;
        }
    }

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
                printf("  Tipo Dia: %s, Variante: %s, Frecuencia: %s, Cod Ubic Parada: %s, Ordinal: %s, Hora: %s, Dia Anterior: %s, Latitud: %s, Longitud: %s\n",
                       vft->tipo_dia, vft->variante, vft->frecuencia, vft->cod_ubic_parada, vft->ordinal, vft->hora, vft->dia_anterior, vft->latitud, vft->longitud);
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
