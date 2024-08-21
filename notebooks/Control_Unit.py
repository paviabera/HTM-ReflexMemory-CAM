from ReflexMemory import ReflexiveMemory
from htm.bindings.sdr import SDR

class Attention:
    def __init__(self, rm, tm, anomaly_scores):
        self.rm = rm
        self.tm = tm
        self.anomaly_scores = anomaly_scores
        self.access_previous = False

    def process(self, current_sdr, next_sdr, iteration, trN, learnFlag):
        if iteration > trN:
            # Check Reflexive Memory (RM) for a prediction
            rm_value, rm_key = self.rm.predict(current_sdr)

            if rm_key and iteration < len(self.anomaly_scores):
                # RM found a prediction
                predicted_sdr = list(map(int, rm_key.split('-')))
                rm_access = True
            else:
                rm_access = False

            if rm_access:
                # Validate RM prediction with the next input
                rm_correct = (predicted_sdr == next_sdr)

                if rm_correct:
                    self.anomaly_scores[iteration] = 0  # No anomaly detected
                else:
                    # Convert current_sdr back to an SDR object before passing
                    current_sdr_obj = SDR(self.tm.getColumnDimensions(), sparse=current_sdr)
                    self.tm.compute(current_sdr_obj, learn=learnFlag)
                    self.anomaly_scores[iteration] = self.tm.anomaly

                self.access_previous = True

            else:
                # Convert current_sdr back to an SDR object before passing
                current_sdr_obj = SDR(self.tm.getColumnDimensions(), sparse=current_sdr)
                self.tm.compute(current_sdr_obj, learn=learnFlag)
                self.anomaly_scores[iteration] = self.tm.anomaly
                self.access_previous = False

        else:
            # Training phase, only use Temporal Memory
            dimensions = self.tm.getColumnDimensions()
            current_sdr_obj = SDR(dimensions)
            current_sdr_obj.fromSparse(current_sdr)  
            self.tm.compute(current_sdr_obj, learn=learnFlag)
            self.anomaly_scores[iteration] = self.tm.anomaly
