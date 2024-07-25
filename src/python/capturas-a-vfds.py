import csv
from datetime import datetime, timedelta

# Archivos de entrada y salida
archivo_gps = 'stm-buses-2024-06-12_00.csv'
archivo_vfd_agrupados = 'orden_vfd.csv'
archivo_revisar = 'a_revisar.csv'

def convertir_hora(hora):
    # Convertir la hora de salida al formato HH:MM
    hora_str = str(hora)
    hora_fmt = hora_str.zfill(5)
    hora_convertida = hora_fmt[:2] + ':' + hora_fmt[2:4]
    # Verificar si la hora convertida es válida
    if int(hora_convertida[:2]) < 24 and int(hora_convertida[3:]) < 60:
        return hora_convertida
    else:
        return None

def leer_archivo_gps(archivo_gps):
    datos_gps = []
    cabecera = None
    with open(archivo_gps, 'r', newline='') as file, open(archivo_revisar, 'w', newline='') as file_revisar:
        
        writer_revisar = csv.writer(file_revisar)        
        reader = csv.reader(file, delimiter=',')
        
        # Leer y guardar la cabecera
        cabecera_original = next(reader)
        
        # Escribir la cabecera en el archivo de revisión
        writer_revisar.writerow(cabecera_original)
                
        for row in reader:
            hora = row[2]
            frecuencia_original = row[2]  # Almacenar la frecuencia original
            # Convertir la hora de salida al formato HH:MM
            hora_convertida = convertir_hora(hora)
            if hora_convertida is None:
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
    segundos_index = hora.rfind(':')
    hora_frecuencia = hora[0:segundos_index].replace(':','') + '0'

    if (hora_frecuencia < frecuencia) :
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
