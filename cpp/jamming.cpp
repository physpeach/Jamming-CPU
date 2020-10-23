#include "../hpp/jamming.hpp"

namespace PhysPeach{
    double L(Jamming* jam){
        return pow(jam->p.packing/jam->phi, 1./(double)D);
    }

    void createJamming(Jamming* jam){
        jam->phi = Phi_init;
        createParticles(&jam->p);
        createCells(&jam->cells, L(jam));
        createLists(&jam->lists, &jam->cells);
        updateCellList(&jam->cells, &jam->lists, L(jam), jam->p.x);
        return;
    }

    void loadSwapMC(Jamming* jam){
        jam->phi = Phi_init;
        
        std::ostringstream posName;
        posName << "../../swapmc/pos/";
        posName << "pos_N" << Np << "_Phi" << Phi_init << "_id" << ID << ".data";
        std::ifstream file;
        file.open(posName.str().c_str());
        createParticles(&jam->p, &file);
        file.close();
        createCells(&jam->cells, L(jam));
        createLists(&jam->lists, &jam->cells);
        updateCellList(&jam->cells, &jam->lists, L(jam), jam->p.x);
        return;
    }

    void loadJamming(Jamming* jam){

        std::ifstream file;

        std::ostringstream jammingName;
        jammingName << "../jammingpoint/jam_N" << Np << "_Phi" << Phi_init << "_id" << ID <<".data";
        file.open(jammingName.str().c_str());
        file >> jam->phi;
        file.close();
        
        std::ostringstream posName;
        posName << "../pos/pos_N" << Np << "_Phi" << Phi_init << "_id" << ID << ".data";
        file.open(posName.str().c_str());
        createParticles(&jam->p, &file);
        file.close();

        createCells(&jam->cells, L(jam));
        createLists(&jam->lists, &jam->cells);
        updateCellList(&jam->cells, &jam->lists, L(jam), jam->p.x);
        return;
    }

    void deleteJamming(Jamming* jam){
        deleteParticles(&jam->p);
        deleteLists(&jam->lists);
        deleteCells(&jam->cells);
        return;
    }

    int fireJamming(Jamming* jam){
        int loop = 0;
        double dt = dt_init;
        double alpha = alpha_init;
        double power;
        bool mustUpdateCellList;

        bool converged = false;
        int cp = 0;
        setZero(jam->p.v, D*Np);
        while(!converged){
            loop++;
            mustUpdateCellList = updateParticles(&jam->p, L(jam), dt, &jam->lists);
            if(mustUpdateCellList){
                updateCellList(&jam->cells, &jam->lists, L(jam), jam->p.x);
            }
            converged = convergedFire(&jam->p);
            power = powerParticles(&jam->p);
            modifyVelocities(&jam->p, alpha);
            if(power < 0){
                setZero(jam->p.v, D*Np);
                alpha = alpha_init;
                dt *= 0.5;
                cp = 0;
            }else{
                cp++;
                if(cp > 5){
                    dt *= 1.1;
                    if(dt > dt_max){
                        dt = dt_max;
                    }
                    alpha *= 0.99;
                    cp = 0;
                }
            }
            if(loop == 1000000){
                std::cout << "dt: " << dt << std::endl;
            }
        }
        return loop;
    }

    int addDphi(Jamming* jam, double dphi){
        static double Lstart = L(jam);
        bool mustUpdateCellList;
        int loop;

        jam->phi += dphi;
        double Lend = L(jam);
        squeezePositions(&jam->p, Lend/Lstart);
        Lstart = Lend;

        mustUpdateCellList = updateMem(&jam->p, Lend);
        if((jam->cells.numOfCellsPerSide > 3 && Lend/(double)jam->cells.numOfCellsPerSide < 2 * a_max) || 2 * a_max < Lend/(double)(1+jam->cells.numOfCellsPerSide)){
            deleteCells(&jam->cells);
            createCells(&jam->cells, Lend);
            mustUpdateCellList = true;
        }
        if(mustUpdateCellList){
            updateCellList(&jam->cells, &jam->lists, Lend, jam->p.x);
        }
        loop = fireJamming(jam);
        return loop;
    }

    double getCloserJamming(Jamming* jam, double dphi){
        int loop = 0;
        std::cout << "    Squeeze from phi = " << jam->phi << " by dphi = " << dphi << std::endl;
        std::cout << "    phi, E, P, loop:" << std::endl;

        double phimem = jam->phi;
        double *xmem;
        xmem = (double*)malloc(D*Np*sizeof(double));
        memcpy(xmem, jam->p.x, D*Np*sizeof(double));

        int aboveJammingCount = 0;
        double Pnow;
        while (aboveJammingCount < 10){
            Pnow = P(&jam->p, L(jam), &jam->lists);
            std::cout << "    " << jam->phi << ", " << U(&jam->p, L(jam), &jam->lists) << ", " << Pnow << ", " << loop << std::endl;
            if(Pnow > 1.0e-8){
                aboveJammingCount++;
                if(dphi < 5.0e-6 && aboveJammingCount == 1){
                    //if only dphi == 1.0e-6 case
                    phimem = jam->phi;
                    memcpy(xmem, jam->p.x, D*Np*sizeof(double));
                }
            }
            if(Pnow < 1.0e-8){
                if(aboveJammingCount > 0){
                    break;
                }
                phimem = jam->phi;
                memcpy(xmem, jam->p.x, D*Np*sizeof(double));
            }
            loop = addDphi(jam, dphi);
        }
        if(aboveJammingcount >= 10){
            jam->phi = phimem;
            memcpy(jam->p.x, xmem, D*Np*sizeof(double));
            free(xmem);
            return 0.1 * dphi;
        }
        else{
            free(xmem);
            return 1.0e-4;
        }
    }
}