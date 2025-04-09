from htm.bindings.sdr import SDR
from htm.algorithms import SpatialPooler
from htm.bindings.algorithms import TemporalMemory
import csv
from htm.encoders.rdse import RDSE, RDSE_Parameters
import time
import traceback
from ReflexMemory import ReflexiveMemory
from ControlUnit import ControlUnit

class AHTM:

  def __init__(self):
    self.rm = None
    self.cu = None
    self.tm_infer_tm = None
    self.tm_infer_rm = None

  def run(self, dataset_path, config):

    self.tm_infer_tm = 0
    self.tm_infer_rm = 0
       
    scalarEncoderParams = RDSE_Parameters()
    scalarEncoderParams.size = config["enc"]["value"]["size"]
    scalarEncoderParams.sparsity = config["enc"]["value"]["sparsity"]
    scalarEncoderParams.resolution = config["enc"]["value"]["resolution"]
    scalarEncoder = RDSE( scalarEncoderParams )
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

    self.rm = ReflexiveMemory( config['reflexSize'] , config['sp']['inputDimensions'], config['sp']['columnDimensions'])
    self.cu = ControlUnit( config['controlThreshold'] )

    try:

        with open(dataset_path, "r") as fin:
            reader = csv.reader(fin)
            headers = next(reader)
            next(reader)
            next(reader)
            count = 0
            for record in reader:
                
                learn_sp = config['sp']['learn']
                learn_tm = config['tm']['learn']
                if count < config['learnRows']:
                    learn_sp = True
                    learn_tm = True

                consumption = float(record[1])
                consumptionBits = scalarEncoder.encode(consumption)
                encoding = SDR( consumptionBits )

                self.cu.compute(encoding, sp, tm, self.rm)

                self.rm.add(encoding)

                activeColumns = SDR( sp.getColumnDimensions() )

                tmp_tm = time.time()
                sp.compute(encoding, learn_sp, activeColumns)
                tm.compute(activeColumns, learn=learn_tm)
                self.tm_infer_tm = self.tm_infer_tm + (time.time() - tmp_tm)

                tmp_tm = time.time()
                self.rm.predict(encoding)
                self.tm_infer_rm = self.tm_infer_rm + (time.time() - tmp_tm)

                count = count + 1

    except Exception as e:
        print(traceback.format_exc())
        print(e)
