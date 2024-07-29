# -*- coding: utf-8 -*-
import sys
import csv
import os
import mmap
from posix_ipc import SharedMemory, ExistentialError

from ctypes import *


lib = cdll.LoadLibrary('./hash_map.so')

# try:
#     lib = CDLL.LoadLibrary('./hash_map.so')
#     print('Biblioteca cargada exitosamente')
# except Exception as e:
#     print('Error al cargar la biblioteca:', e)

class VFT(Structure):
    _fields_ = [
        ("tipo_dia", c_int),
        ("cod_variante", c_char_p),
        ("frecuencia", c_char_p),
        ("cod_ubic_parada", c_char_p),
        ("ordinal", c_char_p),
        ("hora", c_char_p),
        ("dia_anterior", c_char_p),
        ("X", c_char_p),
        ("Y", c_char_p)
    ]

class VFD(Structure):
    _fields_ = [
        ("id", c_char_p),
        ("codigoEmpresa", c_char_p),
        ("frecuencia", c_char_p),
        ("codigoBus", c_char_p),
        ("variante", c_char_p),
        ("linea", c_char_p),
        ("sublinea", c_char_p),
        ("tipoLinea", c_char_p),
        ("destino", c_char_p),
        ("subsistema", c_char_p),
        ("version", c_char_p),
        ("velocidad", c_char_p),
        ("latitud", c_char_p),
        ("longitud", c_char_p),
        ("fecha", c_char_p)
    ]

class Entry(Structure):
    pass

Entry._fields_ = [
    ("key", c_char_p),
    ("vfts", POINTER(POINTER(VFT))),
    ("vfds", POINTER(POINTER(VFD))),
    ("vft_count", c_size_t),
    ("vft_capacity", c_size_t),
    ("vfd_count", c_size_t),
    ("vfd_capacity", c_size_t),
    ("next", POINTER(Entry))
]

class HashMap(Structure):
    _fields_ = [
        ("buckets", POINTER(POINTER(Entry))),
        ("size", c_size_t),
        ("count", c_size_t),
        ("campos_capturas", POINTER(c_char_p)),
        ("campos_horarios", POINTER(c_char_p))
    ]



# Configuración de las funciones
lib.get_campos_capturas.restype = POINTER(c_char_p)
lib.get_campos_capturas.argtypes = [POINTER(HashMap)]

lib.get_campos_horarios.restype = POINTER(c_char_p)
lib.get_campos_horarios.argtypes = [POINTER(HashMap)]

lib.get_capturas.restype = POINTER(POINTER(VFD))
lib.get_capturas.argtypes = [POINTER(Entry)]

lib.get_horarios.restype = POINTER(POINTER(VFT))
lib.get_horarios.argtypes = [POINTER(Entry)]

lib.hash_map_search.restype = POINTER(Entry)
lib.hash_map_search.argtypes = [POINTER(HashMap), c_char_p]

lib.get_all_keys.restype = POINTER(c_char_p)
lib.get_all_keys.argtypes = [POINTER(HashMap), POINTER(c_size_t)]

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

def load_struct_as_layer(list,x_field, y_field, crs,name,campos):
    def getValores(registro, campos):
        valores ={}
        for campo in campos:
            valores.append(registro[campo])
        return valores
    
    layer = QgsVectorLayer(f"Point?crs={crs}", name , "memory")
    if not layer.isValid():
        raise Exception(f"Failed to load layer: {layer.error()}")
    features = []
    fields = QgsFields()
    
    for campo in campos:
        # Define fields
        if (campo == "latitud" or 
            campo == "longitud" or 
            campo == "X" or 
            campo == "Y" ):
            fields.append(QgsField(campo, QVariant.Double))
        elif (campo == 'fecha'):
            fields.append(QgsField(campo, QVariant.DateTime))
        else:
            fields.append(QgsField(campo, QVariant.String))

    layer.dataProvider().addAttributes(fields)
    layer.updateFields()

    for coord in list:
        coord[x_field] = float(coord[x_field].replace(',', '.'))
        coord[y_field] = float(coord[y_field].replace(',', '.'))
        point = QgsPointXY(coord[x_field],coord[y_field])

        feature = QgsFeature()
        feature.setAttributes(getValores(coord, campos))
        feature.setGeometry(QgsGeometry.fromPointXY(point))
        features.append(feature)
    layer.dataProvider().addFeatures(features)
    layer.updateExtents()
    QgsProject.instance().addMapLayer(layer)
    return layer




#BorrarFuncion esta funcion no va a estar mas una vez que el c le pase los diccionarios
def load_list_as_layer(list,x_field, y_field, crs,name):
    layer = QgsVectorLayer(f"Point?crs={crs}", name , "memory")
    if not layer.isValid():
        raise Exception(f"Failed to load layer: {layer.error()}")
    features = []
    fields = QgsFields()
    
    for field in list[0].keys():
        # Define fields
        if (field == "latitud" or 
            field == "longitud" or 
            field == "X" or 
            field == "Y" ):
            fields.append(QgsField(field, QVariant.Double))
        elif (field == 'fecha'):
            fields.append(QgsField(field, QVariant.DateTime))
        else:
            fields.append(QgsField(field, QVariant.String))

    layer.dataProvider().addAttributes(fields)
    layer.updateFields()

    for coord in list:
        coord[x_field] = float(coord[x_field].replace(',', '.'))
        coord[y_field] = float(coord[y_field].replace(',', '.'))
        point = QgsPointXY(coord[x_field],coord[y_field])

        feature = QgsFeature()
        feature.setAttributes([*coord.values()])
        feature.setGeometry(QgsGeometry.fromPointXY(point))
        features.append(feature)
    layer.dataProvider().addFeatures(features)
    layer.updateExtents()
    QgsProject.instance().addMapLayer(layer)
    return layer

# posible mejor
# def load_dict_as_layer(data_dict, x_field, y_field, crs, name):
#     layer = QgsVectorLayer(f"Point?crs={crs}", name, "memory")
#     if not layer.isValid():
#         raise Exception(f"Failed to load layer: {layer.error()}")
    
#     features = []
#     fields = QgsFields()
    
#     # Define fields based on the keys of the dictionary
#     for field in data_dict.keys():
#         if field.lower() in ['latitud', 'longitud', 'x', 'y']:
#             fields.append(QgsField(field, QVariant.Double))
#         elif field.lower() == 'fecha':
#             fields.append(QgsField(field, QVariant.DateTime))
#         else:
#             fields.append(QgsField(field, QVariant.String))
    
#     layer.dataProvider().addAttributes(fields)
#     layer.updateFields()

#     # Create features from the dictionary
#     for key, values in data_dict.items():
#         x_value = float(values[x_field].replace(',', '.')) if isinstance(values[x_field], str) else values[x_field]
#         y_value = float(values[y_field].replace(',', '.')) if isinstance(values[y_field], str) else values[y_field]
        
#         point = QgsPointXY(x_value, y_value)
        
#         feature = QgsFeature()
#         feature.setAttributes([*values.values()])
#         feature.setGeometry(QgsGeometry.fromPointXY(point))
#         features.append(feature)
    
#     layer.dataProvider().addFeatures(features)
#     layer.updateExtents()
#     QgsProject.instance().addMapLayer(layer)
    
#     return layer


# Function to save a layer as a shapefile
def save_layer_as_shapefile(layer, output_path):
    options = QgsVectorFileWriter.SaveVectorOptions()
    options.driverName = 'ESRI Shapefile'
    options.fileEncoding = 'UTF-8'
    error = QgsVectorFileWriter.writeAsVectorFormatV2(layer, output_path, QgsProject.instance().transformContext(), options)
    if error != QgsVectorFileWriter.NoError:
        raise Exception(f"Failed to save shapefile: {output_path}")


def transformar_fechas(f1):
    f1_str = f1.toString('yyyy-MM-dd HH:mm:ss')
    return datetime.strptime(f1_str, '%Y-%m-%d %H:%M:%S')

def determinar_tipo_dia(fecha):
    fecha_obj = datetime.strptime(fecha, '%Y-%m-%d')
    dia_semana = fecha_obj.weekday()
    if dia_semana < 5:
        return 1  # Día Hábil
    elif dia_semana == 5:
        return 2  # Sábado
    else:
        return 3  # Domingo

# Function to calculate estimated passage time
def calcular_hora_estimada(reg_a, reg_b, parada):
    nearest_features = [reg_a, reg_b]
    nearest_features.sort(key=lambda x: x['fecha'])
    
    reg_a = nearest_features[0]
    reg_b = nearest_features[1]

    fecha_a = transformar_fechas(reg_a['fecha'])
    fecha_b = transformar_fechas(reg_b['fecha'])

    # print ("aca van los dos registros ")
    # print(fecha_a)
    # print(fecha_b)

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
            distancia_a_parada = mag_ap
            tiempo_hasta_parada = distancia_a_parada / velocidad_estimada
            hora_estimada = fecha_a - timedelta(seconds=tiempo_hasta_parada)
        elif proyeccion_ap > longitud_vector_ab:
            distancia_b_parada = mag_bp
            tiempo_hasta_parada = distancia_b_parada / velocidad_estimada
            hora_estimada = fecha_b + timedelta(seconds=tiempo_hasta_parada)
        else:
            distancia_a_parada = proyeccion_ap
            tiempo_hasta_parada = distancia_a_parada / velocidad_estimada
            hora_estimada = fecha_a + timedelta(seconds=tiempo_hasta_parada)
    else: 
        raise Exception(f"VELOCIDAD ES 0")

    return hora_estimada

# ruta_vfd por capturas,ruta_vft por horarios,campos_capturas salida
def cargarCapas_y_Calculo_con_C(ruta_vfd,ruta_vft,campos_capturas,campos_horaios, salida):
  
    # funcion para guardar en la salida.csv una fila
    #BorrarFuncion esta funcion no va a estar mas una vez que el c le pase los diccionarios
    def escribir_csv(row):
        with open(salida, 'a', newline='') as file:
            writer = csv.writer(file)
            # Escribir la row
            writer.writerow(row)
    
    # funcion para escribir la fila en un hash de atrasos
    def escribir_hash(row):
        return 0

    #escribe en la cabecera de la salida
    #BorrarFuncion esta funcion no va a estar mas una vez que el c le pase los diccionarios
    escribir_csv(['VFD', 'variante', 'codigo_bus', 'linea', 'hora', 'ordinal','fecha_hora_paso', 'retraso'])

    # Definir el sistema de coordenadas de origen (latitud-longitud, WGS84)
    wgs84 = 'EPSG:4326'  # WGS84
    # Definir el sistema de coordenadas de destino (proyectadas, EPSG específico)
    utm21s = 'EPSG:32721'
    
    # Load list como layer de los struct
    capa_vfd = load_struct_as_layer(ruta_vfd, 'latitud', 'longitud', wgs84,"vfd", campos_capturas)
    capa_vft = load_struct_as_layer(ruta_vft, 'X', 'Y', utm21s,"vft", campos_horaios)
    
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
        'DISTANCE': 250,
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

    # Identify valid stops within the buffer
    paradas_validas = []
    for parada in capa_vft.getFeatures():
        for buffer_feature in capa_buffer.getFeatures():
            if buffer_feature.geometry().contains(parada.geometry()):
                paradas_validas.append(parada)
                print('parada: ', parada['ordinal'])
                break
    
    
    ultimo_registro = ruta_vft[-1]  # Obtener el último valor del diccionario ruta_vft
    ordinal_terminal = ultimo_registro['ordinal']  # Obtener el valor del campo 'ordinal' del último registro
    print(f"Ordinal de el destino en vft: {ordinal_terminal}")
    

    # Iterar sobre las paradas validas en el buffer
    resultados = []
    cont = 0
    for parada_valida in paradas_validas:
        cont+=1
        # Obtener los dos registros más cercanos en capa_vfd
        nearest_ids = spatial_index.nearestNeighbor(parada_valida.geometry(), 2)
        if  len(nearest_ids) < 2:
            print('***************   parada sin vecinos  **********************')
            continue
        else:
            # Verificar que los IDs obtenidos sean diferentes y la distancia entre ellos sea mayor que cero
            reg_a = capa_vfd.getFeature(nearest_ids[0])
            reg_b = capa_vfd.getFeature(nearest_ids[1])
            registroCumputable = True
            
            
            if parada_valida['ordinal'] == ordinal_terminal:  # Lógica para la parada final del VFT
                print('***************   parada final   **********************')
                registros_misma_geometria = [reg_a, reg_b]
                cantidad_iguales = 2
                while True:
                    nearest_ids = spatial_index.nearestNeighbor(parada_valida.geometry().asPoint(), cantidad_iguales)
                    for nearest_id in nearest_ids:
                        reg_c = capa_vfd.getFeature(nearest_id)
                        if reg_c.geometry().equals(reg_a.geometry()):
                            registros_misma_geometria.append(reg_c)
                        else:
                            break
                    if reg_c.geometry().equals(reg_a.geometry()):
                        cantidad_iguales += 1
                        if cantidad_iguales > 40:
                            registroCumputable = False
                            break
                    else:
                        break

                # Seleccionar el registro más temprano en tiempo
                registro_mas_temprano = min(registros_misma_geometria, key=lambda reg: reg['fecha'])
                fecha_hora_estimada = transformar_fechas(registro_mas_temprano['fecha'])  # Calcular la hora estimada de llegada
                
                vfd_string = registro_mas_temprano['VFD']
                fecha_vfd = datetime.strptime(vfd_string.split('_')[-1], '%Y-%m-%d').date()

                if parada_valida['dia_anterior'] == 'S':
                    fecha_vfd += timedelta(days=0)

                hora_vft = str(parada_valida['hora']).zfill(4)
                fecha_hora_prevista = datetime.combine(fecha_vfd, datetime.strptime(hora_vft, '%H%M').time())
                retraso = (fecha_hora_estimada - fecha_hora_prevista).total_seconds() / 60.0
                
            elif parada_valida['ordinal'] == 1: # Lógica para la parada de salida del VFT
                print('##############   parada inicial    #############')
                registros_misma_geometria = [reg_a, reg_b]
                cantidad_iguales = 2
                while True:
                    nearest_ids = spatial_index.nearestNeighbor(parada_valida.geometry().asPoint(), cantidad_iguales)
                    for nearest_id in nearest_ids:
                        reg_c = capa_vfd.getFeature(nearest_id)
                        if reg_c.geometry().equals(reg_a.geometry()):
                            registros_misma_geometria.append(reg_c)
                        else:
                            break
                    if reg_c.geometry().equals(reg_a.geometry()):
                        cantidad_iguales += 1
                        if cantidad_iguales > 40:
                            registroCumputable = False
                            break
                    else:
                        break

                # Seleccionar el registro más tarde en tiempo
                registro_mas_tarde = max(registros_misma_geometria, key=lambda reg: reg['fecha'])
                fecha_hora_estimada = transformar_fechas(registro_mas_tarde['fecha'])  # Calcular la hora estimada de llegada
                
                vfd_string = registro_mas_tarde['VFD']
                fecha_vfd = datetime.strptime(vfd_string.split('_')[-1], '%Y-%m-%d').date()

                if parada_valida['dia_anterior'] == 'S':
                    fecha_vfd += timedelta(days=0)

                hora_vft = str(parada_valida['hora']).zfill(4)
                fecha_hora_prevista = datetime.combine(fecha_vfd, datetime.strptime(hora_vft, '%H%M').time())
                retraso = (fecha_hora_estimada - fecha_hora_prevista).total_seconds() / 60.0

            else :  # Lógica para las paradas intermedias de VFT

                cantidad_iguales = 2
                while reg_a.geometry().asPoint() == reg_b.geometry().asPoint():
                    cantidad_iguales+=1
                    # Si los dos registros están en el mismo lugar, obtener el siguiente más cercano
                    nearest_ids = spatial_index.nearestNeighbor(parada_valida.geometry().asPoint(), cantidad_iguales)
                    reg_a = capa_vfd.getFeature(nearest_ids[0])  # Usar nearest_ids[1] en lugar de nearest_ids[0]
                    reg_b = capa_vfd.getFeature(nearest_ids[len(nearest_ids)-1])  # Usar nearest_ids[2] en lugar de nearest_ids[1]
                    print('vecinos:', nearest_ids, cantidad_iguales)
                    if cantidad_iguales > 40: 
                        print ("me fui porque no encontre y no es destino")
                        registroCumputable = False
                        break
                

                # Access attributes of the feature
                attributesA = reg_a.attributes()
                attributesB = reg_b.attributes()
                if registroCumputable:
                    fecha_hora_estimada = calcular_hora_estimada(reg_a, reg_b, parada_valida)
                    vfd_string = reg_a['VFD']
                    fecha_vfd = datetime.strptime(vfd_string.split('_')[-1], '%Y-%m-%d').date()

                    if parada_valida['dia_anterior'] == 'S':
                        fecha_vfd += timedelta(days=0)

                    hora_vft = str(parada_valida['hora']).zfill(4)
                    fecha_hora_prevista = datetime.combine(fecha_vfd, datetime.strptime(hora_vft, '%H%M').time())
                    retraso = (fecha_hora_estimada - fecha_hora_prevista).total_seconds() / 60.0
                else:
                    vfd_string = reg_a['VFD']
                    fecha_vfd = datetime.strptime(vfd_string.split('_')[-1], '%Y-%m-%d').date()
                    hora_vft = str(parada_valida['hora']).zfill(4)
                    
                    fecha_hora_estimada = transformar_fechas(reg_a['fecha'])
                    
                    if parada_valida['dia_anterior'] == 'S':
                        fecha_vfd += timedelta(days=0)
                    retraso = 100
                

            # retraso = (fecha_hora_estimada - fecha_hora_prevista).total_seconds() / 60.0

            row = []
            row.append(fecha_vfd.strftime("%Y-%m-%d"))
            row.append(reg_a['variante'])
            row.append(reg_a['codigoBus'])
            row.append(reg_a['linea'])
            row.append(hora_vft)
            row.append(parada_valida['ordinal'])
            row.append(fecha_hora_estimada.strftime("%Y-%m-%d %H:%M:%S"))
            row.append(retraso)

            escribir_csv(row)

            resultados.append({
                'VFD': fecha_vfd.strftime("%Y-%m-%d"),
                'codigo_bus': reg_a['codigoBus'],
                'linea': reg_a['linea'],
                'fecha_hora_paso': fecha_hora_estimada.strftime("%Y-%m-%d %H:%M:%S"),
                'retraso': retraso
            })
        
        print('parada_valida', parada_valida['ordinal'])  

    # remueve las layers agregados arriba
    QgsProject.instance().removeMapLayer(QgsProject.instance().mapLayersByName('vft')[0].id())
    QgsProject.instance().removeMapLayer(QgsProject.instance().mapLayersByName('vfd')[0].id())
    QgsProject.instance().removeMapLayer(QgsProject.instance().mapLayersByName('output')[0].id())
    
    # Print the results
    for resultado in resultados:
        print(resultado.values())


#este proceedimento recibe dos diccionario y devuelve en el csv salida los atrasos para cada vfd
def cargarCapas_y_Calculo(ruta_vfd,ruta_vft,salida):
    
    # funcion para guardar en la salida.csv una fila
    #BorrarFuncion esta funcion no va a estar mas una vez que el c le pase los diccionarios
    def escribir_csv(row):
        with open(salida, 'a', newline='') as file:
            writer = csv.writer(file)
            # Escribir la row
            writer.writerow(row)
    
    # funcion para escribir la fila en un hash de atrasos
    def escribir_hash(row):
        return 0

    #escribe en la cabecera de la salida
    #BorrarFuncion esta funcion no va a estar mas una vez que el c le pase los diccionarios
    escribir_csv(['VFD', 'variante', 'codigo_bus', 'linea', 'hora', 'ordinal','fecha_hora_paso', 'retraso'])

    
    
    # Definir el sistema de coordenadas de origen (latitud-longitud, WGS84)
    wgs84 = 'EPSG:4326'  # WGS84
    # Definir el sistema de coordenadas de destino (proyectadas, EPSG específico)
    utm21s = 'EPSG:32721'
    
    # Load list como layer no diccionario
    # BorrarFuncion esto no se va a llamar asi
    capa_vfd = load_list_as_layer(ruta_vfd, 'latitud', 'longitud', wgs84,"vfd")
    capa_vft = load_list_as_layer(ruta_vft, 'X', 'Y', utm21s,"vft")
    
    # Load list como layer de los struct
    
    
    camposVFDs = "traer los campos del struct VFDs"
    camposVFTs = "traer los campos del struct VFTs"
    
    capa_vfd = load_struct_as_layer(ruta_vfd, 'latitud', 'longitud', wgs84,"vfd", camposVFDs)
    capa_vft = load_struct_as_layer(ruta_vft, 'X', 'Y', utm21s,"vft")
    
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
        'DISTANCE': 250,
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

    # Identify valid stops within the buffer
    paradas_validas = []
    for parada in capa_vft.getFeatures():
        for buffer_feature in capa_buffer.getFeatures():
            if buffer_feature.geometry().contains(parada.geometry()):
                paradas_validas.append(parada)
                print('parada: ', parada['ordinal'])
                break
    
    
    ultimo_registro = ruta_vft[-1]  # Obtener el último valor del diccionario ruta_vft
    ordinal_terminal = ultimo_registro['ordinal']  # Obtener el valor del campo 'ordinal' del último registro
    print(f"Ordinal de el destino en vft: {ordinal_terminal}")
    

    # Iterar sobre las paradas veliad en el buffer
    resultados = []
    cont = 0
    for parada_valida in paradas_validas:
        cont+=1
        # Obtener los dos registros más cercanos en capa_vfd
        nearest_ids = spatial_index.nearestNeighbor(parada_valida.geometry(), 2)
        if  len(nearest_ids) < 2:
            print('***************   parada sin vecinos  **********************')
            continue
        else:
            # Verificar que los IDs obtenidos sean diferentes y la distancia entre ellos sea mayor que cero
            reg_a = capa_vfd.getFeature(nearest_ids[0])
            reg_b = capa_vfd.getFeature(nearest_ids[1])
            registroCumputable = True
            
            
            if parada_valida['ordinal'] == ordinal_terminal:  # Lógica para la parada final del VFT
                print('***************   parada final   **********************')
                registros_misma_geometria = [reg_a, reg_b]
                cantidad_iguales = 2
                while True:
                    nearest_ids = spatial_index.nearestNeighbor(parada_valida.geometry().asPoint(), cantidad_iguales)
                    for nearest_id in nearest_ids:
                        reg_c = capa_vfd.getFeature(nearest_id)
                        if reg_c.geometry().equals(reg_a.geometry()):
                            registros_misma_geometria.append(reg_c)
                        else:
                            break
                    if reg_c.geometry().equals(reg_a.geometry()):
                        cantidad_iguales += 1
                        if cantidad_iguales > 40:
                            registroCumputable = False
                            break
                    else:
                        break

                # Seleccionar el registro más temprano en tiempo
                registro_mas_temprano = min(registros_misma_geometria, key=lambda reg: reg['fecha'])
                fecha_hora_estimada = transformar_fechas(registro_mas_temprano['fecha'])  # Calcular la hora estimada de llegada
                
                vfd_string = registro_mas_temprano['VFD']
                fecha_vfd = datetime.strptime(vfd_string.split('_')[-1], '%Y-%m-%d').date()

                if parada_valida['dia_anterior'] == 'S':
                    fecha_vfd += timedelta(days=0)

                hora_vft = str(parada_valida['hora']).zfill(4)
                fecha_hora_prevista = datetime.combine(fecha_vfd, datetime.strptime(hora_vft, '%H%M').time())
                retraso = (fecha_hora_estimada - fecha_hora_prevista).total_seconds() / 60.0
                
            elif parada_valida['ordinal'] == 1: # Lógica para la parada de salida del VFT
                print('##############   parada inicial    #############')
                registros_misma_geometria = [reg_a, reg_b]
                cantidad_iguales = 2
                while True:
                    nearest_ids = spatial_index.nearestNeighbor(parada_valida.geometry().asPoint(), cantidad_iguales)
                    for nearest_id in nearest_ids:
                        reg_c = capa_vfd.getFeature(nearest_id)
                        if reg_c.geometry().equals(reg_a.geometry()):
                            registros_misma_geometria.append(reg_c)
                        else:
                            break
                    if reg_c.geometry().equals(reg_a.geometry()):
                        cantidad_iguales += 1
                        if cantidad_iguales > 40:
                            registroCumputable = False
                            break
                    else:
                        break

                # Seleccionar el registro más tarde en tiempo
                registro_mas_tarde = max(registros_misma_geometria, key=lambda reg: reg['fecha'])
                fecha_hora_estimada = transformar_fechas(registro_mas_tarde['fecha'])  # Calcular la hora estimada de llegada
                
                vfd_string = registro_mas_tarde['VFD']
                fecha_vfd = datetime.strptime(vfd_string.split('_')[-1], '%Y-%m-%d').date()

                if parada_valida['dia_anterior'] == 'S':
                    fecha_vfd += timedelta(days=0)

                hora_vft = str(parada_valida['hora']).zfill(4)
                fecha_hora_prevista = datetime.combine(fecha_vfd, datetime.strptime(hora_vft, '%H%M').time())
                retraso = (fecha_hora_estimada - fecha_hora_prevista).total_seconds() / 60.0

            else :  # Lógica para las paradas intermedias de VFT

                cantidad_iguales = 2
                while reg_a.geometry().asPoint() == reg_b.geometry().asPoint():
                    cantidad_iguales+=1
                    # Si los dos registros están en el mismo lugar, obtener el siguiente más cercano
                    nearest_ids = spatial_index.nearestNeighbor(parada_valida.geometry().asPoint(), cantidad_iguales)
                    reg_a = capa_vfd.getFeature(nearest_ids[0])  # Usar nearest_ids[1] en lugar de nearest_ids[0]
                    reg_b = capa_vfd.getFeature(nearest_ids[len(nearest_ids)-1])  # Usar nearest_ids[2] en lugar de nearest_ids[1]
                    print('vecinos:', nearest_ids, cantidad_iguales)
                    if cantidad_iguales > 40: 
                        print ("me fui porque no encontre y no es destino")
                        registroCumputable = False
                        break
                

                # Access attributes of the feature
                attributesA = reg_a.attributes()
                attributesB = reg_b.attributes()
                if registroCumputable:
                    fecha_hora_estimada = calcular_hora_estimada(reg_a, reg_b, parada_valida)
                    vfd_string = reg_a['VFD']
                    fecha_vfd = datetime.strptime(vfd_string.split('_')[-1], '%Y-%m-%d').date()

                    if parada_valida['dia_anterior'] == 'S':
                        fecha_vfd += timedelta(days=0)

                    hora_vft = str(parada_valida['hora']).zfill(4)
                    fecha_hora_prevista = datetime.combine(fecha_vfd, datetime.strptime(hora_vft, '%H%M').time())
                    retraso = (fecha_hora_estimada - fecha_hora_prevista).total_seconds() / 60.0
                else:
                    vfd_string = reg_a['VFD']
                    fecha_vfd = datetime.strptime(vfd_string.split('_')[-1], '%Y-%m-%d').date()
                    hora_vft = str(parada_valida['hora']).zfill(4)
                    
                    fecha_hora_estimada = transformar_fechas(reg_a['fecha'])
                    
                    if parada_valida['dia_anterior'] == 'S':
                        fecha_vfd += timedelta(days=0)
                    retraso = 100
                

            # retraso = (fecha_hora_estimada - fecha_hora_prevista).total_seconds() / 60.0

            row = []
            row.append(fecha_vfd.strftime("%Y-%m-%d"))
            row.append(reg_a['variante'])
            row.append(reg_a['codigoBus'])
            row.append(reg_a['linea'])
            row.append(hora_vft)
            row.append(parada_valida['ordinal'])
            row.append(fecha_hora_estimada.strftime("%Y-%m-%d %H:%M:%S"))
            row.append(retraso)

            escribir_csv(row)

            resultados.append({
                'VFD': fecha_vfd.strftime("%Y-%m-%d"),
                'codigo_bus': reg_a['codigoBus'],
                'linea': reg_a['linea'],
                'fecha_hora_paso': fecha_hora_estimada.strftime("%Y-%m-%d %H:%M:%S"),
                'retraso': retraso
            })
        
        print('parada_valida', parada_valida['ordinal'])  

    # remueve las layers agregados arriba
    QgsProject.instance().removeMapLayer(QgsProject.instance().mapLayersByName('vft')[0].id())
    QgsProject.instance().removeMapLayer(QgsProject.instance().mapLayersByName('vfd')[0].id())
    QgsProject.instance().removeMapLayer(QgsProject.instance().mapLayersByName('output')[0].id())
    
    # Print the results
    for resultado in resultados:
        print(resultado.values())

#procedimiento que dado VFDs y horarios . consigue los vft y llama a cargadorCapas y calculo
#son ambos archivos csv el archivo_vfd como el arvhivo_vft
#BorrarFuncion esta funcion no va a estar mas una vez que el c le pase los diccionarios
def procesar_Hash(nombreHash):
    salida = os.getcwd() + '/retrasos.csv'  #csv de los atrasos
    # Obtener lista de VFDs (claves del HashMap)

    # key_count = c_size_t()
    # keys_pointer = lib.get_all_keys(nombreHash, byref(key_count))
    # print("-*********************texto raro *************************")
    # listaVfd = [keys_pointer[i].decode('utf-8') for i in range(key_count.value)]


    # listaVfd = get_all_keys(nombreHash)

    # Obtener campos de capturas y horarios
    print(nombreHash)
    campos_capturas = lib.get_campos_capturas(nombreHash)
    print("-*********************texto raro *************************")
    campos_horarios = lib.get_campos_horarios(nombreHash)
    
    # campos_capturas = get_campos_capturas(nombreHash)
    # campos_horarios = get_campos_horarios(nombreHash)
    vfd ="4421_8230_2024-06-09"
    print("el vfd buscado ", vfd)
    entry = lib.hash_map_search(nombreHash, vfd.encode('utf-8'))
    capturas = lib.get_capturas(entry)
    horarios = lib.get_horarios(entry)
    cargarCapas_y_Calculo_con_C(capturas, horarios, campos_capturas, campos_horarios , salida)

    # for vfd in listaVfd:
    #     print("el vfd buscado ", vfd)
    #     entry = lib.hash_map_search(nombreHash, vfd.encode('utf-8'))
    #     capturas = lib.get_capturas(entry)
    #     horarios = lib.get_horarios(entry)
    #     cargarCapas_y_Calculo_con_C(capturas, horarios, campos_capturas, campos_horarios , salida)


        # entry = hash_map_search(nombreHash,vfd)
        # capturas = get_capturas(entry)
        # horarios = get_horarios(entry)
        # cargarCapas_y_Calculo_con_C(capturas, horarios, campos_capturas, campos_horarios , salida)
        



#procedimiento que dado VFDs y horarios . consigue los vft y llama a cargadorCapas y calculo
#son ambos archivos csv el archivo_vfd como el arvhivo_vft
#BorrarFuncion esta funcion no va a estar mas una vez que el c le pase los diccionarios
def procesar_vfd_vft(archivo_vfd, archivo_vft):
    # Diccionario para almacenar los datos agrupados por VFD
    vfd_data = {}

    # Leer y agrupar datos por VFD desde "archivo_vfd"
    with open(archivo_vfd, newline='',encoding='utf-8') as csvfile_vfd:
        reader_vfd = csv.DictReader(csvfile_vfd)
        # next(reader_vfd)
        for row_vfd in reader_vfd:
            vfd = row_vfd['VFD']  # Suponiendo que 'VFD' es el nombre del campo que contiene el VFD

            if (row_vfd['latitud'] != '-0.0' and row_vfd['longitud'] != '-0.0'):
                # Agregar la línea actual al grupo del VFD correspondiente
                if vfd not in vfd_data:
                    vfd_data[vfd] = []
                vfd_data[vfd].append(row_vfd)

    # Procesar cada grupo de VFD y sus VFT correspondientes
    for vfd, registros_vfd in vfd_data.items():
        # Obtener variante, frecuencia y tipo de día del VFD
        variante, frecuencia, fecha = vfd.split('_')
        tipo_dia = determinar_tipo_dia(fecha)  # Determinar el tipo de día según la fecha

        # Buscar todos los VFT correspondientes en "archivo_vft"
        vfts_correspondientes = []
        for row_vft in leer_archivo_vft(archivo_vft):
            if (int(row_vft['cod_variante']) == int (variante) and
                int(row_vft['frecuencia']) == int(frecuencia) and
                int(row_vft['tipo_dia']) == (tipo_dia)):
                # print(row_vft)
                # print(row_vft.values())
                vfts_correspondientes.append(row_vft)

        # Procesar cada combinación de registros_vfd y VFTs correspondientes
        if vfts_correspondientes:
            # Procesar con QGIS los registros_vfd y vfts_correspondientes
                # procesar_con_qgis(registros_vfd, vfts_correspondientes)
                print("---")
                print(registros_vfd[0]['VFD'])
                cargarCapas_y_Calculo(registros_vfd,vfts_correspondientes,salida)
        else:   
            print(f"No se encontraron VFTs correspondientes para el VFD: {vfd}. Saltando procesamiento.")
            
        # para procesar solo el primer vfd uno descomentar la siguiente linea
        # return

#funcion que carga de un csv de los horarios solo carga filas
#BorrarFuncion esta funcion no va a estar mas una vez que el c le pase los diccionarios
def leer_archivo_vft(archivo_vft):
    # Función para leer y retornar datos de "archivo_vft"
    with open(archivo_vft, newline='') as csvfile_vft:
        reader_vft = csv.DictReader(csvfile_vft, delimiter=';')
        # next(reader_vft)
        for row in reader_vft:
            yield row  # Generador para obtener cada fila del archivo VFT


archivo_vfd = os.getcwd() + '/orden_vfd2.csv' #csv de los vfd agrupados opr vfd
archivo_vft = os.getcwd() + '/horarios_paradas_vft.csv' # csv de las paradas para sacar el vfg
salida = os.getcwd() + '/retrasos.csv'  #csv de los atrasos

# procesar_vfd_vft(archivo_vfd, archivo_vft)

# if len(sys.argv) > 1:
#     try:
#         hash_map_address = int(sys.argv[1], 16)
#         print("Dirección recibida en Python:", hex(hash_map_address))
        
#         hash_map = POINTER(HashMap).from_address(hash_map_address)
        
#         # Verificar si el puntero es NULL
#         if hash_map_address == 0:
#             print('Error: El puntero HashMap es NULL')
#         else:
#             print('Dirección del puntero HashMap:', hex(addressof(hash_map.contents)))
#             print('Size:', hash_map.contents.size)
#             print('Count:', hash_map.contents.count)
#     except Exception as e:
#         print('Error al acceder al puntero HashMap:', e)


# hash_map = lib.get_vfd_map()
# hash_map_address = addressof(hash_map.contents)
# print('Dirección del puntero HashMap:', hex(hash_map_address))

# try:
#     hash_map = lib.get_vfd_map()
#     if not hash_map:
#         raise ValueError("Failed to get a valid HashMap pointer")
# except Exception as e:
#         print(f"Error: {e}")

# print(' a ver qu imprime', hash_map)
# print('Size:', hash_map.contents.size)
# print('Count:', hash_map.contents.count)
# hash_map_pointer = POINTER(HashMap).from_address(hash_map)
# print(' a ver qu imprime', hash_map_pointer)

SHM_NAME = '/vfd_map_shm'
SHM_SIZE = 4096  # Adjust size according to your needs

try:
    shm = SharedMemory(SHM_NAME)
    with mmap.mmap(shm.fd, shm.size) as mm:
        # Read the raw data
        raw_data = mm.read(shm.size)

        # Create a ctypes pointer to the shared memory
        hashmap_ptr = cast(raw_data, POINTER(HashMap))
        # Access the contents
        hashmap = hashmap_ptr.contents
        # Print or process the data
        print(f"HashMap size: {hashmap.size}")
        print(f"HashMap count: {hashmap.count}")
        campos_capturas = lib.get_campos_capturas(hashmap)
        print(campos_capturas[0].decode('utf-8'))
except ExistentialError:
    print(f"Shared memory object '{SHM_NAME}' not found.")
except Exception as e:
    print(f"Error: {e}")

# procesar_Hash(hash_map)

# Recibir el argumento como un puntero a HashMap
# nombreHash = int(sys.argv[1], 16)
# hash_map_pointer = POINTER(HashMap).from_address(nombreHash)
# procesar_Hash(hash_map)


# Exit QGIS Application
qgs.exitQgis()
