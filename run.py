from htm.algorithms.anomaly_likelihood import AnomalyLikelihood
import numpy as np
import pandas as pd
import pathlib
from sklearn.metrics import precision_recall_fscore_support
from sklearn.metrics import accuracy_score
from tqdm import tqdm
from sklearn.metrics import roc_auc_score
import json
from AHTM import AHTM

config = None
with open('config.json', 'r') as f:
    config = json.load(f)

dataset_metrics = []

input_path = pathlib.Path(config['inputPath'])
pbar = tqdm(total=len(config['inputSources']))

for dataset in config['inputSources']:

    dataset_path = input_path.joinpath(dataset)

    ahtm = AHTM()
    ahtm.run( dataset_path, config )

    def roc_auc_score_multiclass(y_true, y_pred):
        scores = []
        for y_class in set(y_true):
            y_true_class = [True if x == y_class else False for x in y_true]
            y_pred_class = [True if x == y_class else False for x in y_pred]
            scores.append(roc_auc_score(y_true_class, y_pred_class))
        return sum(scores) / len(scores)

    def match(y, x, idx1, cu, accuracyThreshold):
        n_samples = len(y)
        score1 = cu.anomalyScore(y[idx1], x[idx1])
        if score1 > (1 - accuracyThreshold):
            idx_closest = None
            score_closest = None
            for idx2 in range(n_samples):
                score2 = cu.anomalyScore(y[idx2], x[idx1])
                if score_closest is None or score_closest > score2:
                    score_closest = score2
                    idx_closest = idx2
            return idx_closest
        return idx1

    def calculateMetricsAnomaly(anomaly_scores, config, suffix):

        metric = {}

        anomaly_probability = []
        anomaly_history = AnomalyLikelihood(config["anomaly"]["period"])
        for anomaly_value in anomaly_scores:
            anomaly_probability.append( anomaly_history.compute(anomaly_value) )

        metric['anomaly-avg-'+suffix] = sum(anomaly_scores) / len(anomaly_scores)
        metric['anomaly-samples-'+suffix] = len(anomaly_scores)
        metric['anomaly-prob-avg-'+suffix] = np.count_nonzero(anomaly_probability) / len(anomaly_probability)

        return metric

    def calculateMetrics(Y_dataset, X_dataset, anomaly_scores, total_infe_time, config, cu, suffix):

        metric = {}

        n_samples = len(Y_dataset)

        Y_labels = list(range(n_samples))
        X_labels = [ match(Y_dataset, X_dataset, idx, cu, config['accuracyThreshold']) for idx in range(n_samples)]
        precision, recall, fscore, support = precision_recall_fscore_support(Y_labels, X_labels, average='macro', zero_division=0.0)
        metric['total-infer-time-'+suffix] = total_infe_time
        metric['infer-time-'+suffix] = total_infe_time / len(anomaly_scores)
        metric['accuracy-'+suffix] = accuracy_score(Y_labels, X_labels)
        metric['precision-'+suffix] = precision
        metric['recall-'+suffix] = recall
        metric['fscore-'+suffix] = fscore
        metric['support-'+suffix] = support
        metric['auc-'+suffix] = roc_auc_score_multiclass(Y_labels, X_labels)

        metric.update( calculateMetricsAnomaly(anomaly_scores, config, suffix) )

        return metric
    
    tm_infer_cu = 0
    avg_infer_time_rm = ahtm.tm_infer_rm / len(ahtm.cu.anomalyRM)
    avg_infer_time_tm = ahtm.tm_infer_tm / len(ahtm.cu.anomalyTM)
    tm_infer_cu = tm_infer_cu + (avg_infer_time_rm * ahtm.cu.countRMCU)
    tm_infer_cu = tm_infer_cu + (avg_infer_time_tm * (len(ahtm.cu.anomalyCU) - ahtm.cu.countRMCU))

    metric = {}
    metric['dataset'] = dataset
    metric['cu-rm-count'] = ahtm.cu.countRMCU
    metric.update( calculateMetrics(ahtm.cu.historyGT, ahtm.cu.historyRM, ahtm.cu.anomalyRM, ahtm.tm_infer_rm, config, ahtm.cu, 'rm') )
    metric.update( calculateMetrics(ahtm.cu.historyGT, ahtm.cu.historyTM, ahtm.cu.anomalyTM, ahtm.tm_infer_tm, config, ahtm.cu, 'tm') )
    metric.update( calculateMetrics(ahtm.cu.historyGT, ahtm.cu.historyCU, ahtm.cu.anomalyCU, tm_infer_cu, config, ahtm.cu, 'cu') )
    metric.update( calculateMetricsAnomaly(ahtm.cu.anomalyNU, config, 'nupic') )

    dataset_metrics.append(metric)
    pbar.update(1)

pbar.close()

df = pd.DataFrame(dataset_metrics)

table_1_features = ['dataset','accuracy-cu','accuracy-rm','accuracy-tm','anomaly-avg-cu','anomaly-avg-rm','anomaly-avg-tm','anomaly-avg-nupic']
df[table_1_features].to_csv('metrics-table1.csv', index=False)

table_2_features = ['dataset','anomaly-prob-avg-cu','anomaly-prob-avg-rm','anomaly-prob-avg-tm','anomaly-prob-avg-nupic','anomaly-samples-cu','anomaly-samples-rm','anomaly-samples-tm','anomaly-samples-nupic']
df[table_2_features].to_csv('metrics-table2.csv', index=False)

table_3_features = ['dataset','infer-time-cu','infer-time-rm','infer-time-tm','total-infer-time-cu','total-infer-time-tm','total-infer-time-rm']
df[table_3_features].to_csv('metrics-table3.csv', index=False)

df['cu-infer-speedup'] = 1 - (df['total-infer-time-cu'] / df['total-infer-time-tm'])
df['cu-accuracy-improvement'] = df['accuracy-cu'] - df['accuracy-tm']
df['cu-tm-count'] = df['anomaly-samples-cu'] - df['cu-rm-count']

table_4_features = ['dataset','cu-accuracy-improvement','cu-infer-speedup','cu-rm-count','cu-tm-count','anomaly-samples-cu','infer-time-rm','infer-time-tm']
df[table_4_features].to_csv('metrics-table4.csv', index=False)

table_5_features = ['dataset'] + sorted(list(set(df.columns) - (set(table_1_features) | set(table_2_features) | set(table_3_features)  | set(table_4_features))))
df[table_5_features].to_csv('metrics-table5.csv', index=False)
