from htm.bindings.sdr import SDR
import numpy as np

class ControlUnit:
  def __init__(self, controlThreshold):
    self.anomalyRM = []
    self.anomalyTM = []
    self.anomalyNU = []
    self.anomalyCU = []
    self.historyRM = []
    self.historyTM = []
    self.historyGT = []
    self.historyCU = []
    self.countRMCU = 0
    self.controlThreshold = controlThreshold

  def anomalyScore(self, y, x):
      if np.count_nonzero(y) != 0:
          return 1 - np.count_nonzero((x & y)) / np.count_nonzero(y)
      return 1

  def compute(self, denseColumns1, sp, tm, rm):

    if rm.acKey0 is not None:

      denseColumns0 = SDR( rm.dimensions_dense )
      denseColumns0.sparse = list(map(int, rm.acKey0.split('-')))

      tm.activateDendrites(True)
      predictiveCells = tm.getPredictiveCells()

      predictiveColumns = SDR( rm.dimensions_sparse_sp )
      predictiveColumns.sparse = list(set(sorted(list(np.where(predictiveCells.dense == 1)[0]))))

      reflexiveColumns = SDR( rm.dimensions_sparse_sp )
      reflexiveCount, denseReflexiveColumns = rm.predict(denseColumns0)
      if denseReflexiveColumns is not None:
        sp.compute(denseReflexiveColumns, False, reflexiveColumns)

      activeColumns0 = SDR( rm.dimensions_sparse_sp )
      sp.compute(denseColumns0, False, activeColumns0)

      activeColumns1 = SDR( rm.dimensions_sparse_sp )
      sp.compute(denseColumns1, False, activeColumns1)

      self.historyRM.append( reflexiveColumns.dense )
      self.historyTM.append( predictiveColumns.dense )
      self.historyGT.append( activeColumns1.dense )

      self.anomalyNU.append(tm.anomaly)
      self.anomalyRM.append( self.anomalyScore(activeColumns1.dense, reflexiveColumns.dense) )
      self.anomalyTM.append( self.anomalyScore(activeColumns1.dense, predictiveColumns.dense) )

      if (len(self.anomalyRM) > self.controlThreshold) and (sum(self.anomalyRM[(-1-self.controlThreshold):-1]) > sum(self.anomalyTM[-1-self.controlThreshold:-1])):
        self.anomalyCU.append( self.anomalyTM[-1] )
        self.historyCU.append( self.historyTM[-1] )
        self.countRMCU = self.countRMCU + 1
      else:
        self.anomalyCU.append( self.anomalyRM[-1] )
        self.historyCU.append( self.historyRM[-1] )
