

class ReflexiveMemory:
  def __init__(self):
    self.previous_sdr = None
    self.pairs = {}

  def add(self, sdr):
    current_sdr = '-'.join(map(str, sdr))
    if(self.previous_sdr != None):
      values = self.pairs.get(self.previous_sdr, {})
      pair_count = values.get(current_sdr, 0)
      pair_count = pair_count + 1
      if self.pairs.get(self.previous_sdr, None) is None:
        self.pairs[self.previous_sdr] = { current_sdr: pair_count }
      else:
        self.pairs[self.previous_sdr][current_sdr] = pair_count
    self.previous_sdr = current_sdr

  def predict(self, sdr):
    values = self.pairs.get(sdr, {})
    return_value = 0
    return_key = None
    for key, value in values.items():
      if value > return_value:
        return_value = value
        return_key = key
    return return_value, return_key
