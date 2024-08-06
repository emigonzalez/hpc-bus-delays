#!/bin/bash

# Verificar si se pasa un número como parámetro
if [ $# -ne 1 ]; then
    echo "Uso: $0 <numero de procesos>"
    exit 1
fi

# Parámetro de número de procesos
NUM_PROCESOS=$1

# Definir el rango de IPs y la lista de máquinas válidas
HOSTS=""
for i in $(seq 106 154); do
    HOST="pcunix$i.fing.edu.uy"
    if ping -c 1 -W 1 $HOST &> /dev/null; then
        HOSTS="$HOSTS $HOST"
    fi
done

# Filtrar la cantidad correcta de máquinas y separar por comas
HOSTS=$(echo $HOSTS | awk -v n="$NUM_PROCESOS" '{for(i=1;i<=n;i++) printf("%s,", $i)}' | sed 's/,$//')

# Generar el comando mpirun
if [ -n "$HOSTS" ]; then
    echo "mpirun -np $NUM_PROCESOS -hosts $HOSTS ./main"
else
    echo "No se encontraron máquinas válidas en el rango especificado."
fi
