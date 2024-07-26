import csv
from datetime import datetime, timedelta

# Archivos de entrada y salida
archivo_gps = 'stm-buses-2024-07-16_00.csv'
archivo_vfd_agrupados = 'orden_vfd.csv'
archivo_revisar = 'a_revisar.csv'

def convertir_frecuencia_a_minutos(frecuencia):
    frecuencia_n = int(frecuencia) / 10
    hora = int(frecuencia_n / 100)
    minutos = int(frecuencia_n % 100)

    # Verificar si la frecuencia es válida
    if hora < 24 and minutos < 60:
        frecuencia_convertida = hora * 60 + minutos
        return frecuencia_convertida
    else:
        return None

def convertir_hora_a_frecuencia(hora):
    segundos_index = hora.rfind(':')
    hora_frecuencia = hora[0:segundos_index].replace(':','') + '0'
    return hora_frecuencia

def leer_archivo_gps(archivo_gps):
    datos_gps = []
    with open(archivo_gps, 'r', newline='') as file, open(archivo_revisar, 'w', newline='') as file_revisar:
        
        writer_revisar = csv.writer(file_revisar)        
        reader = csv.reader(file, delimiter=',')
        
        # Leer y guardar la cabecera
        cabecera_original = next(reader)
        
        # Escribir la cabecera en el archivo de revisión
        writer_revisar.writerow(cabecera_original)
                
        for row in reader:
            frecuencia_original = row[2]  # Almacenar la frecuencia original
            # dia = row[17].split()[0]   # 2024-06-11
            hora = row[17].split()[1]  # 00:39:33
            latitud = row[15]
            longitud = row[16]
            # Convertir la hora de salida al formato HH:MM
            frecuencia_convertida = convertir_frecuencia_a_minutos(frecuencia_original)
            hora_convertida = convertir_hora_a_frecuencia(hora)
            hora_convertida = convertir_frecuencia_a_minutos(hora_convertida)
            diferencia = 0 if frecuencia_convertida is None else abs(frecuencia_convertida - hora_convertida)

            print(f"f: {frecuencia_original} -> {frecuencia_convertida}, h: {hora_convertida}, d: {diferencia}")

            if (
                frecuencia_convertida is None or # 65535
                (diferencia > (3*60) and diferencia < (21*60)) or # mas de 3 horas
                latitud == "0" or longitud == "0"
            ):
                writer_revisar.writerow(row)
            else:
                row[2] = frecuencia_original  # Mantener el formato original de la frecuencia
                datos_gps.append(row)
    return datos_gps, cabecera_original

def generar_vfd(registro):
    # Generar el VFD combinando el código de variante, la frecuencia y el día
    variante = registro[4]
    frecuencia = registro[2]        # 23550
    dia = registro[17].split()[0]   # 2024-06-11
    hora = registro[17].split()[1]  # 00:39:33
    frecuencia_convertida = convertir_frecuencia_a_minutos(frecuencia)
    hora_convertida = convertir_hora_a_frecuencia(hora)
    hora_convertida = convertir_frecuencia_a_minutos(hora_convertida)

    if (frecuencia_convertida - hora_convertida > 20) :
        dia = (datetime.strptime(dia, '%Y-%m-%d') - timedelta(days=1)).strftime('%Y-%m-%d')

    vfd = variante + '_' + frecuencia + '_' + dia
    return vfd

def agrupar_por_vfd(datos_gps):
    vfd_dict = {}
    vfd_actual = None
    primer_registro_vfd = None

    for registro in datos_gps:
        vfd = generar_vfd(registro)

        if vfd != vfd_actual:
            # Si encontramos un nuevo VFD, guardamos el primer registro
            vfd_actual = vfd
            primer_registro_vfd = registro

        if vfd not in vfd_dict:
            vfd_dict[vfd] = []

        # Añadir el registro al VFD correspondiente
        vfd_dict[vfd].append(registro)

    return vfd_dict, primer_registro_vfd

def escribir_csv(datos_agrupados, archivo_vfd_agrupados, cabecera_original):
    with open(archivo_vfd_agrupados, 'w', newline='') as file:
        writer = csv.writer(file)
        # Escribir la cabecera
        cabecera = ['VFD'] + cabecera_original
        writer.writerow(cabecera)
        
        for vfd, registros in datos_agrupados.items():
            for registro in registros:
                # Insertar el VFD como primer elemento del registro
                registro.insert(0, vfd)
                writer.writerow(registro)

# Leer datos del archivo GPS
datos_gps, cabecera_original = leer_archivo_gps(archivo_gps)

# Agrupar por VFD
datos_agrupados, _ = agrupar_por_vfd(datos_gps)

# Escribir el resultado en archivos CSV
escribir_csv(datos_agrupados, archivo_vfd_agrupados, cabecera_original)
print("Proceso completado. Se ha generado el archivo:", archivo_vfd_agrupados)
