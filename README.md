# Programación paralela en HPC en C usando MPI
## Retrasos de buses de STM
La idea es calcular el retraso de los autobuses en Montevideo durante el mes de junio de 2024. Cada hora se genero un archivo CSV con las capturas de las ubicaciones de los autobuses (latitud, longitud) durante todo el mes cada 20 segundos. También hay un archivo con los horarios de los autobuses por parada. El cálculo del retraso se realiza en Python utilizando los algoritmos de buffer y vecinos más cercanos de QGIS.

Las ubicaciones de los autobuses se capturaron durante el mes de junio de 2024 y se almacenan en un archivo por hora por día. Esto significa que hay 24 archivos por hora en 30 días.

Tanto las ubicaciones como los horarios, se almacenan en un directorio por dia en la ruta data/capturas/2024-06-01 y data/horarios/2024-06-01.

### Pasos

1. **Inicialización y Configuración**:
    - Inicializar MPI.
    - Cada proceso conoce el número total de procesos y su propio rango.
2. **Distribuir Nombres de Archivos**:
    - Crear una lista de bloques (dias del mes).
    - Distribuir los dias entre los procesos.
3. **Lectura de Archivos**:
    - Cada proceso lee de forma independiente sus archivos asignados y carga los datos.
4. **Agrupación de Datos**:
    - Para cada dia, se agrupar los datos de ubicación VFD.
    - Para cada dia, se agrupar los datos de horarios VFT.
5. **Mapeo de Ubicaciones a Horarios**:
    - Cada proceso mapea sus datos de ubicación agrupados a los datos de horarios agrupados.
6. **Calcular Retrasos**:
    - Pasar los resultados del mapeo al algoritmo de "caja negra" en Python para calcular el retraso.
7. **Recopilar Resultados**:
    - Recopilar los resultados de todos los procesos y sumarizar retrasos.
8. **Post Procesamiento**:
    - Agrupar datos de ventas de boletos, asociarlos a los atrasos y almacenarlos en un archivo.

### Puntos clave
 1. **Distribución de Bloques**: El proceso master distribuye los bloques utilizando `MPI_Scatter`.
 2. **Lectura Independiente de Archivos**: Cada proceso lee de forma independiente sus archivos asignados, lo que reduce la necesidad de comunicación entre procesos en esta etapa.
 3. **Agrupación y Mapeo**: Implementar estructuras de datos eficientes como lo hash maps para manejar las operaciones de agrupación y mapeo.
 4. **Integración con Python**: Asegurar una interacción fluida entre C y Python para los cálculos de retraso.
 5. **Recopilación de Resultados**: Se utiliza sincronizacion con `MPI_Barrier` y envios bloqueantes con `MPI_Ssend` para enviar los resultados al proceso master.

### Ejecutar el proceso

```shell
# Cargar modulo mpi
module load mpi/mpich-x86_64
# Para compilar el sistema
make clean && make
# Correr n procesos en el mismo nodo
mpirun –np <num> ./main
# Correr n procesos en diferentes nodos
mpirun –np <num> -hosts pcunix140,pcunix142 ./main
# Tambien puedes especificar un hosts file
mpirun -np <num> --hostfile hosts-file.txt ./main
```
