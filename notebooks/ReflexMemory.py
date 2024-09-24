from htm.bindings.sdr import SDR, Metrics
import numpy as np

class ReflexiveMemory:
  def __init__(self,dimensions):
    self.previous_sdr = None
    self.pairs = {}
    self.dimensions = dimensions

  def add(self, sdr):
    # pair_count = 0
    current_sdr = '-'.join(map(str, sdr.sparse))
    if(self.previous_sdr != None):
      values = self.pairs.get(self.previous_sdr, {})
      pair_count = values.get(current_sdr, 0)
      pair_count = pair_count + 1
      if self.pairs.get(self.previous_sdr, None) is None:
        self.pairs[self.previous_sdr] = { current_sdr: pair_count }
      else:
        self.pairs[self.previous_sdr][current_sdr] = pair_count
    self.previous_sdr = current_sdr
    print("Pair count",pair_count)

  def predict(self, sdr):
    search_sdr = '-'.join(map(str, sdr.sparse))
    values = self.pairs.get(search_sdr, {})
    return_value = 0
    return_key = None
    for key, value in values.items():
      if value > return_value:
        return_value = value
        return_key = key
    if return_key is not None:
      return_sdr = SDR( self.dimensions )
      return_sdr.sparse = list(map(int, return_key.split('-')))
      return_key = return_sdr
    return return_value, return_key
  
 # Control Unit
  def learn(self, sdr, SMactiveColumns):
    pred_correct = False
    pred_anomaly = None
    if self.previous_sdr is not None:
        prev_activeColumns = SDR( self.dimensions )
        prev_activeColumns.sparse = list(map(int, self.previous_sdr.split('-')))

        pred_value, pred_key = self.predict(prev_activeColumns)
        if pred_key is not None:
            if pred_key.flatten() == sdr.flatten():
                pred_correct = True
                pred_anomaly = 0
            else:
                key1 = self.previous_sdr
                key2 = '-'.join(map(str, pred_key.sparse))
                self.pairs[key1][key2] = pred_value - 1
                
                pred_anomaly = 1 - np.count_nonzero((pred_key.dense & sdr.dense)) / np.count_nonzero(sdr.dense)
    # print("pred_anomaly",pred_anomaly)
    return pred_correct, pred_anomaly
