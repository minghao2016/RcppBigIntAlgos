#include "SieveUtils.h"

// Getting quadratic residues. See tonellishanks.cc for more
// details. The array "TS" was used here to make the code
// more concise and since everything will be stored in
// SieveDist, TS can easily be cleared from memory when done.
std::vector<std::size_t> setSieveDist(mpz_t myNum, mpz_t *const TS,
                                      const std::vector<std::size_t> &facBase) {
    
    const std::size_t facSize = facBase.size();
    std::vector<std::size_t> SieveDist(facSize * 2, 0u);
    SieveDist[0] = SieveDist[1] = 1;
    
    mpz_t p;
    mpz_init(p);
    
    for (std::size_t i = 1, row = 2; i < facSize; ++i, row += 2) {
        mpz_set_ui(p, facBase[i]);
        TonelliShanksC(myNum, p, TS);
        
        SieveDist[row] = mpz_get_ui(TS[1]);
        SieveDist[row + 1] = mpz_get_ui(TS[2]);
    }
    
    mpz_clear(p);
    return SieveDist;
}

std::vector<std::size_t> outersect(std::vector<std::size_t> &x, std::vector<std::size_t> &y) {
    
    std::sort(x.begin(), x.end());
    std::sort(y.begin(), y.end());
    
    const std::size_t lenX = x.size();
    const std::size_t lenY = y.size();
    std::vector<std::size_t> v(lenX + lenY);
    
    auto it = std::set_difference(x.begin(), x.end(), y.begin(), y.end(), v.begin());
    auto it2 = std::set_difference(y.begin(), y.end(), x.begin(), x.end(), it);
    v.resize(it2 - v.begin());
    
    return v;
}

std::vector<std::uint8_t> myIntToBit(std::size_t x, std::size_t dig) {
    
    std::vector<std::uint8_t> binaryVec(dig);
    
    for (std::size_t i = 0; x > 0; ++i) {
        binaryVec[i] = x % 2;
        x >>= 1;
    }
    
    return binaryVec;
}

std::vector<std::size_t> getPrimesQuadRes(mpz_t myN, double LimB, double fudge1,
                                          double sqrLogLog, std::size_t myTarget) {
    
    const std::size_t uN = LimB;
    std::vector<char> primes(uN + 1, 1);
    std::vector<std::size_t> myps;
    
    myps.reserve(LimB * 2.0 / std::log(LimB));
    const std::size_t fsqr = std::floor(std::sqrt(LimB));
    
    for (std::size_t j = 4; j <= uN; j += 2)
        primes[j] = 0;
    
    for (std::size_t lastP = 3; lastP <= fsqr;) {
        for (std::size_t j = lastP * lastP; j <= uN; j += 2 * lastP)
            primes[j] = 0;
        
        std::size_t k = lastP + 2;
        std::size_t ind = 2;
        
        while (!primes[k]) {
            k += 2;
            ind += 2;
        }
        
        lastP += ind;
    }
    
    mpz_t test, jmpz, temp;
    mpz_init(test);
    mpz_init(jmpz);
    mpz_init(temp);
    
    myps.push_back(2u);
    
    for (std::size_t j = 3; j <= uN; j += 2) {
        if (primes[j]) {
            mpz_set_si(jmpz, j);
            mpz_set_si(temp, j);
            mpz_sub_ui(temp, jmpz, 1);
            mpz_div_2exp(temp, temp, 1);
            mpz_powm(test,myN,temp,jmpz);
            
            if (mpz_cmp_ui(test, 1) == 0)
                myps.push_back(j);
        }
    }
    
    mpz_clear(jmpz); mpz_clear(temp);
    mpz_clear(test);
    
    mpz_t currP, nextP, resTest, CP1;
    mpz_init(currP); mpz_init(nextP);
    mpz_init(CP1); mpz_init(resTest);
    
    while (myps.size() < myTarget) {
        fudge1 += 0.005;
        LimB = std::exp((0.5 + fudge1) * sqrLogLog);
        mpz_set_ui(currP, myps.back());
        mpz_nextprime(nextP, currP);
        
        while (mpz_cmp_ui(nextP, LimB) < 0) {
            mpz_set(currP, nextP);
            mpz_nextprime(nextP, currP);
            mpz_sub_ui(CP1, currP, 1);
            mpz_div_2exp(CP1, CP1, 1);
            mpz_powm(resTest, myN, CP1, currP);
            
            if (mpz_cmp_ui(resTest, 1) == 0)
                myps.push_back(mpz_get_ui(currP));
        }
    }
    
    mpz_clear(currP); mpz_clear(nextP);
    mpz_clear(CP1); mpz_clear(resTest);
    return myps;
}

void sieveListsInit(const std::vector<std::size_t> &FBase, const std::vector<double> &LnFB,
                    const std::vector<std::size_t> &polySieveD, std::vector<double> &myLogs,
                    std::vector<std::size_t> &myStart, mpz_t firstSqrDiff,
                    mpz_t lowerBound, std::size_t strt) {
    
    mpz_t modTest;
    mpz_init(modTest);
    const std::size_t vecMaxSize = myLogs.size();
    
    for (std::size_t i = strt, row = strt * 2, facSize = FBase.size(); i < facSize; ++i, row += 2) {
        mpz_mod_ui(modTest, lowerBound, FBase[i]);
        const std::int64_t q = mpz_get_si(modTest);
        
        mpz_mod_ui(modTest, firstSqrDiff, FBase[i]);
        std::int64_t myStart0 = mpz_get_si(modTest);
        
        std::int64_t myStart1 = 0;
        std::int64_t myMin = 0;
        std::int64_t myMax = 0;
        
        if (polySieveD[row] > polySieveD[row + 1]) {
            myMax = polySieveD[row];
            myMin = polySieveD[row + 1];
        } else {
            myMin = polySieveD[row];
            myMax = polySieveD[row + 1];
        }
        
        if (myStart0 == 0) {
            if (q == myMin) {
                myStart1 = (myMax - myMin);
            } else {
                myStart1 = FBase[i] - (myMax - myMin);
            }
        } else {
            const std::int64_t signedFB = FBase[i];
            
            if (myMin > q) {
                myStart0 = myMin - q;
            } else if (mpz_sgn(lowerBound) < 0) {
                myStart0 = -1 * (q - signedFB - myMin);
            } else {
                myStart0 = signedFB - ((myMax + q) % signedFB);
            }
            
            if (myMax > q) {
                myStart1 = myMax - q;
            } else if (mpz_sgn(lowerBound) < 0) {
                myStart1 = -1 * (q - signedFB - myMax);
            } else {
                myStart1 = signedFB - ((myMin + q) % signedFB);
            }
        }
        
        for (std::size_t j = myStart0; j < vecMaxSize; j += FBase[i])
            myLogs[j] += LnFB[i];
        
        for (std::size_t j = myStart1; j < vecMaxSize; j += FBase[i])
            myLogs[j] += LnFB[i];
        
        myStart[row] = (FBase[i] * ((vecMaxSize - myStart0 + FBase[i] - 1u) / FBase[i]) + myStart0) % vecMaxSize;
        myStart[row + 1] = (FBase[i] * ((vecMaxSize - myStart1 + FBase[i] - 1u) / FBase[i]) + myStart1) % vecMaxSize;
    }
    
    mpz_clear(modTest);
}

void sieveLists(const std::vector<std::size_t> &FBase, const std::vector<double> &LnFB,
                std::vector<std::size_t> &myStart, std::vector<double> &myLogs,
                std::size_t strt) {
    
    std::fill(myLogs.begin(), myLogs.end(), 0.0);
    const std::size_t vecMaxSize = myLogs.size();
    
    for (std::size_t i = strt, row = strt * 2, facSize = FBase.size(); i < facSize; ++i, row += 2) {
        for (std::size_t j = myStart[row]; j < vecMaxSize; j += FBase[i])
            myLogs[j] += LnFB[i];
        
        for (std::size_t j = myStart[row + 1]; j < vecMaxSize; j += FBase[i])
            myLogs[j] += LnFB[i];
        
        myStart[row] = (FBase[i] * ((vecMaxSize - myStart[row] + FBase[i] - 1u) / FBase[i]) + myStart[row]) % vecMaxSize;
        myStart[row + 1] = (FBase[i] * ((vecMaxSize - myStart[row + 1] + FBase[i] - 1u) / FBase[i]) + myStart[row + 1]) % vecMaxSize;
    }
}

void sieveListsFinal(const std::vector<std::size_t> &FBase, const std::vector<double> &LnFB,
                     const std::vector<std::size_t> &myStart, std::vector<double> &myLogs,
                     std::size_t strt) {
    
    std::fill(myLogs.begin(), myLogs.end(), 0.0);
    const std::size_t vecMaxSize = myLogs.size();
    
    for (std::size_t i = strt, row = strt * 2, facSize = FBase.size(); i < facSize; ++i, row += 2) {
        for (std::size_t j = myStart[row]; j < vecMaxSize; j += FBase[i])
            myLogs[j] += LnFB[i];
        
        for (std::size_t j = myStart[row + 1]; j < vecMaxSize; j += FBase[i])
            myLogs[j] += LnFB[i];
    }
}

void SinglePoly(std::vector<std::size_t> &polySieveD, mpz_t *const smoothInterval,
                const std::vector<std::size_t> &SieveDist, mpz_t *const TS,
                const std::vector<std::size_t> &facBase, mpz_t *const mpzFacBase,
                const std::vector<double> &LnFB, mpz_t *const largeCoFactors,
                std::vector<std::size_t> &myStart, mpz_t *const partialInterval,
                vec2dint &powsOfSmooths, vec2dint &powsOfPartials,
                hash64vec &partFactorsMap, hash64mpz_t &partIntvlMap,
                hash64size_t &keepingTrack, std::vector<std::size_t> &coFactorIndexVec,
                std::size_t &nPartial, std::size_t &nSmooth, std::size_t &coFactorInd,
                mpz_t intVal, mpz_t myNum, mpz_t Atemp, mpz_t Btemp, mpz_t temp,
                mpz_t A, mpz_t B, mpz_t C, mpz_t Atemp2, mpz_t lowBound,
                double theCut, std::size_t DoubleLenB, std::size_t mpzFacSize,
                std::size_t vecMaxSize, std::size_t strt) {
    
    TonelliShanksC(myNum, Atemp, TS);
    
    if (mpz_cmp(TS[1], TS[2]) > 0) {
        mpz_set(Btemp, TS[1]);
    } else {
        mpz_set(Btemp, TS[2]);
    }
    
    mpz_mul_2exp(temp, Btemp, 1);
    mpz_invert(temp, temp, Atemp);
    mpz_pow_ui(B, Btemp, 2u);
    
    mpz_sub(B, myNum, B);
    mpz_mul(B, B, temp);
    mpz_add(B, B, Btemp);
    
    mpz_pow_ui(A, Atemp, 2u);
    mpz_mod(B, B, A);
    
    mpz_pow_ui(C, B, 2u);
    mpz_sub(C, C, myNum);
    mpz_divexact(C, C, A);
    
    for (std::size_t i = strt, row = strt * 2,
         facSize = facBase.size(); i < facSize; ++i, row += 2) {
        mpz_invert(Atemp2, A, mpzFacBase[i]);
        
        mpz_ui_sub(temp, SieveDist[row], B);
        mpz_mul(temp, temp, Atemp2);
        mpz_mod_ui(temp, temp, facBase[i]);
        polySieveD[row] = mpz_get_ui(temp);
        
        mpz_ui_sub(temp, SieveDist[row + 1], B);
        mpz_mul(temp, temp, Atemp2);
        mpz_mod_ui(temp, temp, facBase[i]);
        polySieveD[row + 1] = mpz_get_ui(temp);
    }
    
    const int intLowBound = mpz_get_si(lowBound);
    
    mpz_mul_si(temp, B, 2 * intLowBound);
    mpz_add(temp, temp, C);
    mpz_mul_si(Atemp2, A, intLowBound);
    mpz_mul_si(Atemp2, Atemp2, intLowBound);
    mpz_add(temp, Atemp2, temp);
    
    std::vector<double> myLogs(vecMaxSize, 0.0);
    
    sieveListsInit(facBase, LnFB, polySieveD,
                   myLogs, myStart, temp, lowBound, strt);
    
    for (std::size_t chunk = 0; chunk < DoubleLenB; chunk += vecMaxSize) {
        std::vector<std::size_t> largeLogs;
        
        for (std::size_t i = 0, myLogSize = myLogs.size(); i < myLogSize; ++i)
            if (myLogs[i] > theCut)
                largeLogs.push_back(i + chunk);
        
        for (auto lrglog: largeLogs) {
            std::vector<int> primeIndexVec;
            const int myIntVal = intLowBound + lrglog;
            
            mpz_mul_si(temp, B, 2 * myIntVal);
            mpz_add(temp, temp, C);
            mpz_mul_si(Atemp2, A, myIntVal);
            mpz_mul_si(Atemp2, Atemp2, myIntVal);
            mpz_add(intVal, Atemp2, temp);
            
            // Add the index referring to A^2.. (i.e. add it twice)
            primeIndexVec.insert(primeIndexVec.end(), 2, static_cast<int>(mpzFacSize));
            
            // If Negative, we push zero (i.e. the index referring to -1)
            if (mpz_sgn(intVal) < 0) {
                mpz_abs(intVal, intVal);
                primeIndexVec.push_back(0);
            }
            
            for (std::size_t i = 0, facSize = facBase.size(); i < facSize; ++i) {
                while (mpz_divisible_ui_p(intVal, facBase[i])) {
                    mpz_divexact_ui(intVal, intVal, facBase[i]);
                    primeIndexVec.push_back(i + 1);
                }
            }
            
            mpz_mul_si(temp, A, myIntVal);
            
            if (mpz_cmp_ui(intVal, 1u) == 0) {
                // Found a smooth number
                mpz_add(smoothInterval[nSmooth], temp, B);
                powsOfSmooths.push_back(primeIndexVec);
                ++nSmooth;
            } else if (mpz_cmp_d(intVal, Significand53) < 0) {
                const uint64_t myKey = static_cast<uint64_t>(mpz_get_d(intVal));
                const auto pFacIt = partFactorsMap.find(myKey);
                
                if (pFacIt != partFactorsMap.end()) {
                    const auto trackIt = keepingTrack.find(myKey);
                    
                    if (trackIt != keepingTrack.end()) {
                        coFactorIndexVec.push_back(trackIt->second);
                    } else {
                        keepingTrack[myKey] = coFactorInd;
                        mpz_set(largeCoFactors[coFactorInd], intVal);
                        coFactorIndexVec.push_back(coFactorInd);
                        ++coFactorInd;
                    }
                    
                    primeIndexVec.insert(primeIndexVec.begin(),
                                         pFacIt->second.cbegin(), pFacIt->second.cend());
                    
                    powsOfPartials.push_back(primeIndexVec);
                    const auto intervalIt = partIntvlMap.find(myKey);
                    
                    mpz_add(temp, temp, B);
                    mpz_mul(partialInterval[nPartial],
                            temp, intervalIt->second);
                    
                    partFactorsMap.erase(pFacIt);
                    partIntvlMap.erase(intervalIt);
                    ++nPartial;
                } else {
                    partFactorsMap[myKey] = primeIndexVec;
                    mpz_init(partIntvlMap[myKey]);
                    mpz_add(partIntvlMap[myKey], temp, B);
                }
            }
        }
        
        if (chunk + 2 * vecMaxSize < DoubleLenB) {
            sieveLists(facBase, LnFB, myStart, myLogs, strt);
        } else if (chunk + vecMaxSize < DoubleLenB) {
            myLogs.resize(DoubleLenB % vecMaxSize);
            sieveListsFinal(facBase, LnFB, myStart, myLogs, strt);
        }
    }
}