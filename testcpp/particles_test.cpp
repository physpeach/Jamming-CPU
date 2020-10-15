#include "../testhpp/particles_test.hpp"

namespace PhysPeach{
    void createParticlesTest(){

        PhysPeach::Particles p;
        createParticles(&p, 3);

        double diamav = 0.;
        double xav[D];
        for(int d = 0; d < D; d++){
            xav[d] = 0.;
        }
        for(int par1 = 0; par1 < Np; par1++){
            diamav += p.diam[par1];
            for(int d = 0; d < D; d++){
                xav[d] += p.x[d*Np+par1];
            }
        }

        diamav /= Np;
        assert(diamav > 0.99);
        assert(diamav < 1.01);

        double L = pow(p.packing/Phi_init, 1./(double)D);
        for(int d = 0; d < D; d++){
            xav[d] /= Np * L;
            assert(xav[d] > -0.01);
            assert(xav[d] < 0.01);
        }

        for(int par1 = 0; par1 < D*Np; par1++){
            assert(p.x[par1] == p.mem[par1]);
        }
        deleteParticles(&p);
        return;
    }
    void squeezePositionsTest(){
        PhysPeach::Particles p;
        createParticles(&p, 3);
        for(int par1 = 0; par1 < D*Np; par1++){
            p.x[par1] = 1.;
            p.mem[par1] = 1.;
        }
        squeezePositions(&p, 0.99);
        for(int par1 = 0; par1 < D*Np; par1++){
            assert(p.x[par1] == 1. * 0.99);
            assert(p.mem[par1] = 1. * 0.99);
        }
        deleteParticles(&p);
    }
}