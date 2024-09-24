from htm.bindings.sdr import SDR, Metrics
from htm.encoders.rdse import RDSE, RDSE_Parameters
from htm.encoders.date import DateEncoder
from htm.algorithms import SpatialPooler
from htm.bindings.algorithms import TemporalMemory
from htm.algorithms.anomaly_likelihood import AnomalyLikelihood
import numpy as np
import pandas as pd
import datetime
import copy
import pathlib
import csv
import matplotlib.pyplot as plt
import hashlib
from config import config
from ReflexMemory import ReflexiveMemory
from Control_Unit import Attention


def parse_date(date_str):
    formats = {
        7: "%Y-%m",                    # Format: yyyy-mm
        10: "%Y-%m-%d",                # Format: yyyy-mm-dd
        19: "%Y-%m-%d %H:%M:%S"        # Format: yyyy-mm-dd hh-mm-ss
    }

    date_format = formats.get(len(date_str))

    if date_format:
        return datetime.datetime.strptime(date_str, date_format)
    else:
        raise ValueError(f"Date format not recognized for: {date_str}")
    

def run_algorithm(dataset):
    input_path = pathlib.Path('/workspaces/HTM/datasets/numenta')
    records = []
    with open(input_path.joinpath(dataset), "r") as fin:
        reader = csv.reader(fin)
        headers = next(reader)
        next(reader)
        next(reader)
        for record in reader:
            records.append(record)
    
    dateEncoder = DateEncoder(
        timeOfDay= config["enc"]["time"]["timeOfDay"], 
        weekend  = config["enc"]["time"]["weekend"]
    )
    
    scalarEncoderParams = RDSE_Parameters()
    scalarEncoderParams.size = config["enc"]["value"]["size"]
    scalarEncoderParams.sparsity = config["enc"]["value"]["sparsity"]
    scalarEncoderParams.resolution = config["enc"]["value"]["resolution"]
    scalarEncoder = RDSE( scalarEncoderParams )
    # encodingWidth = (dateEncoder.size + scalarEncoder.size)
    encodingWidth = (scalarEncoder.size)


    config['sp']['inputDimensions'] = (encodingWidth,)
    config['sp']['potentialRadius'] = encodingWidth

    sp = SpatialPooler(
        inputDimensions = config['sp']['inputDimensions'],
        columnDimensions = config['sp']['columnDimensions'],
        potentialPct = config['sp']['potentialPct'],
        potentialRadius = config['sp']['potentialRadius'],
        globalInhibition = config['sp']['globalInhibition'],
        localAreaDensity = config['sp']['localAreaDensity'],
        synPermInactiveDec = config['sp']['synPermInactiveDec'],
        synPermActiveInc = config['sp']['synPermActiveInc'],
        synPermConnected = config['sp']['synPermConnected'],
        boostStrength = config['sp']['boostStrength'],
        wrapAround = config['sp']['wrapAround'],
        seed = config['sp']['seed']
    )

    tm = TemporalMemory(
        columnDimensions = config['sp']['columnDimensions'],
        cellsPerColumn = config['tm']['cellsPerColumn'],
        activationThreshold = config['tm']['activationThreshold'],
        initialPermanence = config['tm']['initialPermanence'],
        connectedPermanence = config['sp']['synPermConnected'],
        minThreshold = config['tm']['minThreshold'],
        maxNewSynapseCount = config['tm']['maxNewSynapseCount'],
        permanenceIncrement = config['tm']['permanenceIncrement'],
        permanenceDecrement = config['tm']['permanenceDecrement'],
        predictedSegmentDecrement = config['tm']['predictedSegmentDecrement'],
        maxSegmentsPerCell = config['tm']['maxSegmentsPerCell'],
        maxSynapsesPerSegment = config['tm']['maxSynapsesPerSegment']
    )

    rm = ReflexiveMemory(sp.getColumnDimensions())
    # attention = Attention(rm, tm, [])

    enc_info = Metrics( [encodingWidth], 999999999)
    sp_info = Metrics( sp.getColumnDimensions(), 999999999 )
    tm_info = Metrics( [tm.numberOfCells()], 999999999 )
    anomaly_history = AnomalyLikelihood(config["anomaly"]["period"])

    inputs = []
    anomaly = []
    anomalyProb = []
    AnomalyMean = 0
    AnomalyStd = 0
    anomalyRM = []

    for count, record in enumerate(records):

            # Dynamically determine the date format
        try:
            dateString = parse_date(record[0])

       
            consumption = float(record[1])
            inputs.append( consumption )
            
            dateBits = dateEncoder.encode(dateString)
            consumptionBits = scalarEncoder.encode(consumption)

            # encoding = SDR( encodingWidth ).concatenate([consumptionBits, dateBits])
            encoding = SDR( consumptionBits )
            enc_info.addData( encoding )
            
            activeColumns = SDR( sp.getColumnDimensions() )
            predictiveColumns = SDR( sp.getColumnDimensions() )

            if count < config['learnRows']:
                sp.compute(encoding, True, activeColumns)
                # sp.compute(encoding, config['sp']['learn'], activeColumns)
                sp_info.addData( activeColumns )

                tm.compute(activeColumns, learn=True)
                tm_info.addData( tm.getActiveCells().flatten() )
            else: 

                sp.compute(encoding, config['sp']['learn'], activeColumns)
                sp_info.addData( activeColumns )


                tm.activateDendrites(True)
                predictiveColumns.sparse = list(set(sorted(list(np.where(tm.getPredictiveCells().dense == 1)[0]))))

                pred_correct, pred_anomaly = rm.learn(activeColumns, predictiveColumns)
                anomalyRM.append( pred_anomaly )
                rm.add(activeColumns)
                tm.compute(activeColumns, learn=config['tm']['learn'])
                tm_info.addData( tm.getActiveCells().flatten() )
            # tm.activateDendrites(True)
            # predictiveColumns.sparse = list(set(sorted(list(np.where(tm.getPredictiveCells().dense == 1)[0]))))

           

            # rm.add(copy.deepcopy(activeColumns.sparse))

            # tm.compute(activeColumns, learn=config['tm']['learn'])
            # tm_info.addData( tm.getActiveCells().flatten() )

            # Predict the next input using the attention module
            # if count < len(records) - 1:
            #     next_record = records[count + 1]
            #     next_dateString = parse_date(next_record[0])
            #     next_consumption = float(next_record[1])
            #     next_dateBits = dateEncoder.encode(next_dateString)
            #     next_consumptionBits = scalarEncoder.encode(next_consumption)
            #     next_encoding = SDR(next_consumptionBits)       
                
            #     attention.process(activeColumns.sparse, next_encoding.sparse, count, config['sp']['learnRows'], config['tm']['learn'])
            
            anomaly.append( tm.anomaly )
            anomalyProb.append( anomaly_history.compute(tm.anomaly) )

        except ValueError as e:
            print(f"Error parsing date at record {count}: {e}")
    AnomalyMean = np.mean(anomaly)
    AnomalyStd = np.std(AnomalyStd)
    print(AnomalyMean,AnomalyStd)
    return AnomalyMean, AnomalyStd