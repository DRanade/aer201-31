import numpy as np
import scipy
from random import randint
from scipy import linalg
numSol = 3      # change number here
numObj = 3      # change number here
rpArr = [[ [0 for j in range(numSol)] for i in range(numSol) ] for i in range(numObj)]
rpArrOvPref = [ [ 0 for j in range(numSol)] for i in range(numObj)]
nrpArr = [[ [0 for i in range(numSol)] for i in range(numSol) ] for i in range(numObj)]

for i in range(numObj):
    for j in range(numSol):
        rpArr[i][j][j] = 1.0
        for k in range(numSol-1,j,-1):
            rpArr[i][j][k] = float(raw_input("How do you rate alternative "+str(j+1)+" compared to alternative "+str(k+1)+" with respect to Objective "+str(i+1)+"?"))
            rpArr[i][k][j] = 1/rpArr[i][j][k]
    for j in range(numSol): # normalizes columns
        sumCol = 0
        for k in range(numSol):
            sumCol += rpArr[i][k][j]
        for k in range(numSol):
            nrpArr[i][k][j] = rpArr[i][k][j]/sumCol
            rpArrOvPref[i][j] += nrpArr[i][j][k]

objArr = [ [0 for j in range(numObj)] for i in range(numObj) ]
nobjArr = [ [0 for j in range(numObj)] for i in range(numObj) ]
objArrOvPref = [ 0 for j in range(numObj)]
for j in range(numObj):
    objArr[j][j] = 1.0
    for k in range(numObj-1,j,-1):
        objArr[j][k] = float(raw_input("How do you rate objective "+str(j+1)+" compared to objective "+str(k+1)+"?"))
        objArr[k][j] = 1/objArr[j][k]
for j in range(numObj): # normalizes columns
    sumCol = 0
    for k in range(numObj):
        sumCol += objArr[k][j]
    for k in range(numObj):
        nobjArr[k][j] = objArr[k][j]/sumCol
        objArrOvPref[j] += objArr[j][k]

Dvalues = [0 for i in range(numSol)]
for j in range(len(Dvalues)):
    accum = 0
    for i in range(numObj):
        accum += objArrOvPref[i]*rpArrOvPref[i][j]
    Dvalues[j] = accum

CIs_RPs = [0 for i in range(numObj)]
CRs_RPs = [0 for i in range(numObj)]
CI_RI = 0
CR_RI = 0
qDict = {2:0.00, 3:0.52, 4:0.9, 5:1.12, 6:1.24, 7:1.32, 8:1.41}

for i in range(numObj):
    CIs_RPs[i] = (max(scipy.linalg.eig(rpArr[i])[0])-numSol)/(numSol-1)
    CRs_RPs[i] = CIs_RPs[i]/qDict[numSol]
CI_RI = (max(scipy.linalg.eig(objArr)[0])-numObj)/(numObj-1)
CR_RI = CI_RI/qDict[numObj]

print np.array(CRs_RPs)
print CR_RI
