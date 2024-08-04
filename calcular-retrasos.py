# -*- coding: utf-8 -*-
import sys
import csv
import os
import warnings
warnings.filterwarnings('ignore')
# Configurar las variables de entorno necesarias maquina gabriel

os.environ['QGIS_PREFIX_PATH'] = "/usr"
os.environ['QT_QPA_PLATFORM'] = 'offscreen'  # Configuración para modo sin pantalla
os.environ['PYTHONPATH'] = '/usr/share'
os.environ['LD_LIBRARY_PATH'] = '/usr/share/qgis/python/plugins/'

# Configurar las variables de entorno necesarias maquina facultad


from PyQt5.QtCore import QVariant
from datetime import datetime, timedelta
from qgis.core import (
    QgsApplication,
    QgsProject,
    QgsSpatialIndex,
    QgsVectorLayer,
    QgsFeature,
    QgsGeometry,
    QgsDistanceArea,
    QgsVectorFileWriter,
    QgsCoordinateReferenceSystem,
    QgsPoint,
    QgsPointXY,
    QgsFields,QgsField    
)

# Inicializar QgsApplication
QgsApplication.setPrefixPath('/usr', True)
qgs = QgsApplication([], False)
qgs.initQgis()

sys.path.append("/usr/share/qgis/python/plugins/")

import processing
from qgis.analysis import QgsNativeAlgorithms
from processing.core.Processing import Processing
Processing.initialize()

QgsApplication.processingRegistry().addProvider(QgsNativeAlgorithms())


def load_list_as_layer(list,x_field, y_field, crs,name):
    warnings.filterwarnings('ignore')
    layer = QgsVectorLayer(f"Point?crs={crs}", name , "memory")
    if not layer.isValid():
        raise Exception(f"Failed to load layer: {layer.error()}")
    features = []
    fields = QgsFields()
    
    for field in list[0].keys():
        # Define fields
        if field.lower() in ["latitud", "longitud", "x", "y"]:
            fields.append(QgsField(field, QVariant.Double))
        elif field.lower() == 'fecha':
            fields.append(QgsField(field, QVariant.DateTime))
        else:
            fields.append(QgsField(field, QVariant.String))

    layer.dataProvider().addAttributes(fields)
    layer.updateFields()

    for coord in list:
        x = float(coord[x_field].replace(',', '.'))
        y = float(coord[y_field].replace(',', '.'))
        point = QgsPointXY(x, y)
        feature = QgsFeature()
        feature.setAttributes([*coord.values()])
        feature.setGeometry(QgsGeometry.fromPointXY(point))
        features.append(feature)
 
    layer.dataProvider().addFeatures(features)
    layer.updateExtents()
    QgsProject.instance().addMapLayer(layer)
    return layer

def transformar_fechas(f1):
    f1_str = f1.toString('yyyy-MM-dd HH:mm:ss')
    return datetime.strptime(f1_str, '%Y-%m-%d %H:%M:%S')

# Function to calculate estimated passage time
def calcular_hora_estimada(reg_a, reg_b, parada):
    nearest_features = [reg_a, reg_b]
    nearest_features.sort(key=lambda x: x['fecha'])
    
    reg_a = nearest_features[0]
    reg_b = nearest_features[1]

    fecha_a = transformar_fechas(reg_a['fecha'])
    fecha_b = transformar_fechas(reg_b['fecha'])

    point_a = reg_a.geometry().asPoint()
    point_b = reg_b.geometry().asPoint()
    point_p = parada.geometry().asPoint()

    distancia = QgsDistanceArea().measureLine(point_a, point_b)
    tiempo_transcurrido = (fecha_b - fecha_a).total_seconds()
    velocidad_estimada = distancia / tiempo_transcurrido if tiempo_transcurrido > 0 else 0

    vector_ab = (point_b.x() - point_a.x(), point_b.y() - point_a.y())
    vector_ap = (point_p.x() - point_a.x(), point_p.y() - point_a.y())
    vector_bp = (point_p.x() - point_b.x(), point_p.y() - point_b.y())

    # Calcular el producto punto AB dot AP
    ab_dot_ap = vector_ab[0] * vector_ap[0] + vector_ab[1] * vector_ap[1]

    # Calcular la magnitud de AB y AP
    mag_ab = (vector_ab[0] ** 2 + vector_ab[1] ** 2) ** 0.5
    mag_ap = (vector_ap[0] ** 2 + vector_ap[1] ** 2) ** 0.5
    mag_bp = (vector_bp[0] ** 2 + vector_bp[1] ** 2) ** 0.5

    # Calcular la proyección de AP sobre AB
    proyeccion_ap = ab_dot_ap / mag_ab
    longitud_vector_ab = mag_ab

    if velocidad_estimada > 0:
        if proyeccion_ap < 0:
            distancia_a_parada = proyeccion_ap
            tiempo_hasta_parada = distancia_a_parada / velocidad_estimada
            hora_estimada = fecha_a - timedelta(seconds=tiempo_hasta_parada)
        elif proyeccion_ap > longitud_vector_ab:
            distancia_b_parada = proyeccion_ap
            tiempo_hasta_parada = distancia_b_parada / velocidad_estimada
            hora_estimada = fecha_b + timedelta(seconds=tiempo_hasta_parada)
        else:
            distancia_a_parada = proyeccion_ap
            tiempo_hasta_parada = distancia_a_parada / velocidad_estimada
            hora_estimada = fecha_a + timedelta(seconds=tiempo_hasta_parada)
    else: 
        raise Exception(f"VELOCIDAD ES 0")

    return hora_estimada

def escribir_csv(row):
        with open(salida, 'a', newline='') as file:
            writer = csv.writer(file)
            # Escribir la row
            writer.writerow(row)

#este proceedimento recibe dos diccionario y devuelve en el csv salida los atrasos para cada vfd
def cargarCapas_y_Calculo(capturas,horarios,salida):
    global VFD
    
    # Definir el sistema de coordenadas de origen (latitud-longitud, WGS84)
    wgs84 = 'EPSG:4326'  # WGS84
    # Definir el sistema de coordenadas de destino (proyectadas, EPSG específico)
    utm21s = 'EPSG:32721'
    
    # Load list como layer no diccionario
    capa_vfd = load_list_as_layer(capturas, 'latitud', 'longitud', wgs84,"vfd")
    capa_vft = load_list_as_layer(horarios, 'X', 'Y', utm21s,"vft")
    
    # Reproject vfd temporarily to EPSG:32721
    vfd_temporal = 'memory:'
    reproject_params = {
        'INPUT': capa_vfd,
        'TARGET_CRS': QgsCoordinateReferenceSystem('EPSG:32721'),
        'OUTPUT': vfd_temporal
    }
    reproject_result = processing.run("native:reprojectlayer", reproject_params)
    capa_vfd = reproject_result['OUTPUT']

    spatial_index = QgsSpatialIndex(capa_vfd.getFeatures())

    # Generate a X-meter buffer around VFD points
    buffer_output_path = 'memory:'
    buffer_params = {
        'INPUT': capa_vfd,
        'DISTANCE': 100,
        'SEGMENTS': 5,
        'END_CAP_STYLE': 0,
        'JOIN_STYLE': 0,
        'MITER_LIMIT': 2,
        'DISSOLVE': False,
        'OUTPUT': buffer_output_path
    }
    buffer_result = processing.run("native:buffer", buffer_params)
    capa_buffer = buffer_result['OUTPUT']
    QgsProject.instance().addMapLayer(capa_buffer)

    # identificar paradas validas dentro del buffer
    paradas_validas = []
    for parada in capa_vft.getFeatures():
        for buffer_feature in capa_buffer.getFeatures():
            if buffer_feature.geometry().contains(parada.geometry()):
                paradas_validas.append(parada)
                # print('paradas validas: ', parada['ordinal'])
                break
    
    ultimo_registro = horarios[-1]  # Obtener el último valor del diccionario horarios
    ordinal_terminal = ultimo_registro['ordinal']  # Obtener el valor del campo 'ordinal' del último registro
    # print(f"Ordinal de el destino en vft: {ordinal_terminal}")

    # Iterar sobre las paradas validas en el buffer
    resultados = []
    cont = 0
    for parada_valida in paradas_validas:
        cont+=1
        # Obtener los dos registros más cercanos en capa_vfd
        nearest_ids = spatial_index.nearestNeighbor(parada_valida.geometry(), 2)
        if  len(nearest_ids) < 2:
            # print('***************   parada sin vecinos  **********************')
            continue
        else:
            # Verificar que los IDs obtenidos sean diferentes y la distancia entre ellos sea mayor que cero
            reg_a = capa_vfd.getFeature(nearest_ids[0])
            reg_b = capa_vfd.getFeature(nearest_ids[-1])
            registroCumputable = True
            
            registros_misma_geometria = [reg_a, reg_b]
            cantidad_iguales = len(nearest_ids)  
            
            while reg_a.geometry().asPoint() == reg_b.geometry().asPoint():
            # while True:
                cantidad_iguales+=1
                # Si los dos registros están en el mismo lugar, obtener el siguiente más cercano
                nearest_ids = spatial_index.nearestNeighbor(parada_valida.geometry().asPoint(), cantidad_iguales)
                reg_a = capa_vfd.getFeature(nearest_ids[0])  # Usar nearest_ids[1] en lugar de nearest_ids[0]
                reg_b = capa_vfd.getFeature(nearest_ids[len(nearest_ids)-1])  # Usar nearest_ids[2] en lugar de nearest_ids[1]

                # for nearest_id in nearest_ids:
                #     reg_c = capa_vfd.getFeature(nearest_id)
                #     if reg_c.geometry().equals(reg_a.geometry()):
                #         registros_misma_geometria.append(reg_c)
                #     else:
                #         break
                # if reg_c.geometry().equals(reg_a.geometry()):
                #     cantidad_iguales += 1
                if cantidad_iguales > 10:
                        registroCumputable = False
                        break
                # else:
                #         break
                # nearest_ids = spatial_index.nearestNeighbor(parada_valida.geometry().asPoint(), cantidad_iguales)
                
            # print('vecinos:', nearest_ids, cantidad_iguales)
            if registroCumputable:
                # registro_mas_temprano = min(registros_misma_geometria, key=lambda reg: reg['fecha'])
                # registro_mas_tarde = max(registros_misma_geometria, key=lambda reg: reg['fecha'])
                # if parada_valida['ordinal'] == ordinal_terminal:  # Lógica para la parada final del VFT
                #     print('***************   parada final   **********************')
                #     # Seleccionar el registro más temprano en tiempo
                #     # print('vecinos:', nearest_ids, cantidad_iguales)
                #     reg_a = registro_mas_temprano
                # elif parada_valida['ordinal'] == "1": # Lógica para la parada de salida del VFT
                #     reg_a = registro_mas_tarde
                #     # print('##############   parada inicial    #############')
                # else :  # Lógica para las paradas intermedias de VFT
                #     reg_a = reg_a
            
                fecha_hora_estimada = calcular_hora_estimada(reg_a, reg_b, parada_valida)
                  
                vfd_string = VFD
                fecha_vfd = datetime.strptime(vfd_string.split('_')[-1], '%Y-%m-%d').date()
                if parada_valida['dia_anterior'] in [ 'S', '*']:
                    fecha_vfd += timedelta(days=1)

                hora_vft = str(parada_valida['hora']).zfill(4)
                fecha_hora_prevista = datetime.combine(fecha_vfd, datetime.strptime(hora_vft, '%H%M').time())
                retraso = (fecha_hora_estimada - fecha_hora_prevista).total_seconds() / 60.0

                row = [
                    VFD, 
                    reg_a['variante'],
                    reg_a['codigoBus'],
                    reg_a['linea'],
                    hora_vft,
                    parada_valida['ordinal'],
                    fecha_hora_estimada.strftime("%Y-%m-%d %H:%M:%S"),
                    retraso,
                    cantidad_iguales,
                    parada_valida['cod_ubic_parada'],
                    parada_valida['X'],
                    parada_valida['Y'],
            ]
                escribir_csv(row)
            else:
                continue
        # print('parada procesada: ', parada_valida['ordinal'],' #vecinos: ', cantidad_iguales) 
        # print('vecinos:', nearest_ids, cantidad_iguales)

    # remueve las layers agregados arriba
    QgsProject.instance().removeMapLayer(QgsProject.instance().mapLayersByName('vft')[0].id())
    QgsProject.instance().removeMapLayer(QgsProject.instance().mapLayersByName('vfd')[0].id())
    QgsProject.instance().removeMapLayer(QgsProject.instance().mapLayersByName('output')[0].id())


# procedimiento que dado 3 arvhivos uno de vfd(resumen), capturas y horarios
# toma de captura y horarios las filas correspondientes que el primer archivo le indica de un vfd en particular 
# son ambos archivos csv el archivo_vfd como el arvhivo_vft
def procesar_archivos_retornar_atrasos(archivo_vfd, archivo_capturas,archivo_horarios):
    # Índices iniciales para las filas de capturas y horarios
    global VFD
    escribir_csv(['VFD', 'variante', 'codigo_bus', 'linea', 'hora', 'ordinal','fecha_hora_paso', 'retraso', 'vecinos','cod_parada', 'X', 'Y' ])    
    with open(vfd_file, newline='') as vfdfile:
        vfd_reader = csv.reader(vfdfile)
        next(vfd_reader)  # Saltar la cabecera
        captura_index = 0
        horario_index = 0
        for row in vfd_reader:
            VFD = row[0]
            cant_capturas = int(row[1])
            cant_horarios = int(row[2])
            # Extraer las líneas correspondientes de capturas y horarios
            capturas_dict = []
            horarios_dict = []

            # Asignar las capturas
            if captura_index + cant_capturas <= len(capturas_data):
                capturas_dict.extend(capturas_data[captura_index:captura_index + cant_capturas]) 
                captura_index += cant_capturas 
            else:
                capturas_dict.extend = capturas_data[captura_index:]
                captura_index = len(capturas_data)  # Finaliza el índice si no hay más datos
            # Asignar los horarios
            if horario_index + cant_horarios <= len(horarios_data):
                horarios_dict.extend(horarios_data[horario_index:horario_index + cant_horarios]) 
                horario_index += cant_horarios
            else:
                horarios_dict.extend = horarios_data[horario_index:]
                horario_index = len(horarios_data)  # Finaliza
            # print(VFD)
            # if VFD =='7884_14100_2024-06-10':
            #     cargarCapas_y_Calculo(capturas_dict, horarios_dict, salida) 
            #     break
            cargarCapas_y_Calculo(capturas_dict, horarios_dict, salida) 
            # para procesar solo el primer vfd uno descomentar la siguiente linea
            # return

#  Función para leer el archivo CSV los datos como dictionario
def read_csv(file_path, delimiter=','):
    with open(file_path, newline='') as csvfile:
        reader = csv.DictReader(csvfile, delimiter=delimiter)
        data = list(reader)
    return data

# VFD=""

fechaProcesar = sys.argv[2]
dondeCorrer = sys.argv[1]

if dondeCorrer == 'v':
    salida = os.getcwd() + '/data/retrasos.csv'  #csv de los atrasos
    capturas_file = os.getcwd() + '/data/capturas.csv' #csv de las capturas ordenas por vfd
    horarios_file = os.getcwd() + '/data/horarios.csv' # csv de las paradas ordenadas por vfd
    vfd_file      = os.getcwd() + '/data/vfd.csv' # csv de los vfd ,cant_capturas, cant_horarios
else:
    salida = os.getcwd() + f'/data/retrasos/retrasos_{fechaProcesar}.csv'  #csv de los atrasos
    capturas_file = os.getcwd() + f'/data/temp/capturas_{fechaProcesar}.csv' #csv de las capturas ordenas por vfd
    horarios_file = os.getcwd() + f'/data/temp/horarios_{fechaProcesar}.csv' # csv de las paradas ordenadas por vfd
    vfd_file      = os.getcwd() + f'/data/temp/vfd_{fechaProcesar}.csv' # csv de los vfd ,cant_capturas, cant_horarios

# Leer archivo con coma como delimitador
capturas_data = read_csv(capturas_file, delimiter=',')
# Leer archivo con punto y coma como delimitador
horarios_data = read_csv(horarios_file, delimiter=';')

procesar_archivos_retornar_atrasos(vfd_file,capturas_file,horarios_file)

# Exit QGIS Application
qgs.exitQgis()
