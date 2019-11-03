// -*-c++-*-

/*
 *Copyright:

 Copyright (C) Hiroki SHIMORA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3, or (at your option)
 any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this code; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 *EndCopyright:
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "sample_field_evaluator.h"

#include "field_analyzer.h"
#include "simple_pass_checker.h"

#include <rcsc/player/player_evaluator.h>
#include <rcsc/common/server_param.h>
#include <rcsc/common/logger.h>
#include <rcsc/math_util.h>

#include <iostream>
#include <algorithm>
#include <cmath>
#include <cfloat>

// #define DEBUG_PRINT

using namespace rcsc;

static const int VALID_PLAYER_THRESHOLD = 8;


/*-------------------------------------------------------------------*/
/*!

 */
static double evaluate_state( const PredictState & state );


/*-------------------------------------------------------------------*/
/*!

 */
SampleFieldEvaluator::SampleFieldEvaluator()
{

}

/*-------------------------------------------------------------------*/
/*!

 */
SampleFieldEvaluator::~SampleFieldEvaluator()
{

}

/*-------------------------------------------------------------------*/
/*!

 */
double
SampleFieldEvaluator::operator()( const PredictState & state,
                                  const std::vector< ActionStatePair > & /*path*/ ) const
{
    const double final_state_evaluation = evaluate_state( state );

    //
    // ???
    //

    double result = final_state_evaluation;

    return result;
}


/*-------------------------------------------------------------------*/
/*!

 */
static
double
evaluate_state( const PredictState & state, const rcsc::WorldModel & wm )
{
    /*          Seletiva Robocin - Desafio 4 - Tomada de decisão
    
            O processo de tomada de decisão do agente é um processo complexo, 
        passa por uma geração de possíveis ações, projeção do ambiente ao 
        ser feita essa ação e avaliação do estado, a ação que gera o estado 
        com melhor avaliação é tomada. Modificar a função de avaliação do 
        estado é o que altera o comportamento do agente. Sua função é modificar
        o comportamento do agente em função do tempo da simulação.
    */
    const ServerParam & SP = ServerParam::i();

    const AbstractPlayerObject * holder = state.ballHolder();

#ifdef DEBUG_PRINT
    dlog.addText( Logger::ACTION_CHAIN,
                  "========= (evaluate_state) ==========" );
#endif

    //
    // if holder is invalid, return bad evaluation
    //
    if ( ! holder )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ACTION_CHAIN,
                      "(eval) XXX null holder" );
#endif
        return - DBL_MAX / 2.0;
    }

    const int holder_unum = holder->unum();


    //
    // ball is in opponent goal
    //
    if ( state.ball().pos().x > + ( SP.pitchHalfLength() - 0.1 )
         && state.ball().pos().absY() < SP.goalHalfWidth() + 2.0 )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ACTION_CHAIN,
                      "(eval) *** in opponent goal" );
#endif
        return +1.0e+7;
    }

    //
    // ball is in our goal
    //
    if ( state.ball().pos().x < - ( SP.pitchHalfLength() - 0.1 )
         && state.ball().pos().absY() < SP.goalHalfWidth() )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ACTION_CHAIN,
                      "(eval) XXX in our goal" );
#endif

        return -1.0e+7;
    }


    //
    // out of pitch
    //
    if ( state.ball().pos().absX() > SP.pitchHalfLength()
         || state.ball().pos().absY() > SP.pitchHalfWidth() )
    {
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ACTION_CHAIN,
                      "(eval) XXX out of pitch" );
#endif

        return - DBL_MAX / 2.0;
    }


    //
    // set basic evaluation
    //
    double point = state.ball().pos().x;

    Vector2D bestPoint = ServerParam::i().theirTeamGoalPos();

    int forward_opp = 0;

    for (PlayerPtrCont::const_iterator p = wm.opponentsFromSelf().begin();
         p != wm.opponentsFromSelf().end();
         p++) 
    {
        if((*p)->pos().x > state.self().pos().x && !(*p)->goalie()) {

        }
    }


    if(forward_opp > 0) {
        for(PlayerPtrCont::const_iterator tm = wm.teammatesFromSelf().begin();
            tm != wm.teammatesFromSelf().end();
            tm++)
        {
            // checar se ta impedido
            if( (*tm)->pos().x > wm.offsideLineX()+1.0 ) continue;

            // checar se tem menos gente marcando o teammate

            double tm_x = (*tm)->pos().x;
            
            // depois checar se caso o passe seja feito, nao ocorra interceptacao
        }
    }


    Vector2D left (wm.self().pos().x, wm.self.pos().y-10.0);
    Vector2D right (wm.self().pos().x, wm.self.pos().y+10.0);
    Sector2D sector(wm.self().pos(), 0.0, 7.5, left.th(), right.th());

    int shadow_opp = 0;
    // conta marcaçao num raio de 7.5 a frente do player
    for (PlayerPtrCont::const_iterator p = wm.opponentsFromSelf().begin();
            p != wm.opponentsFromSelf().end();
            p++) 
    {
        if(sector.contains((*p)->pos()) && !(*p)->goalie()) {
            shadow_opp++;
        }
    }

    if(shadow_opp == 0) {
        // continua com o gol como melhor ponto
    }
    else  {
        int less_shadow_opp = 1000;
        Vector2D pos_best_tm;
        for (PlayerPtrCont::const_iterator tm = wm.teammatesFromSelf().begin();
            tm != wm.teammatesFromSelf().end();
            tm++) 
        {
            int tm_shadow_opp = 0;
            // analisa teammates
            Vector2D left_tm ((*tm)->pos().x, (*tm)->pos().y-10.0);
            Vector2D right_tm ((*tm)->pos().x, (*tm)->pos().y+10.0);
            Sector2D sector_tm ((*tm)->pos(), 0.0, 7.5, left_tm.th(), right_tm.th());
            for (PlayerPtrCont::const_iterator op_tm = wm.opponentsFromSelf().begin();
                op_tm != wm.opponentsFromSelf().end();
                op_tm++) 
            {
                if( sector_tm.contains((*op_tm)->pos()) && !(*op_tm)->goalie() ) {
                    tm_shadow_opp++;
                }
            }
            // se tiver menos marcadores e tiver a frente do meio campo
            if( tm_shadow_opp < less_shadow_opp && (std::fabs((*tm)->pos().x) > 0.0) ) {
                less_shadow_opp = tm_shadow_opp;
                pos_best_tm = (*tm)->pos();
            }
        }
        if(less_shadow_opp < shadow_opp) {
            bestPoint = pos_best_tm;
        }
    }

    point += std::max( 0.0,
                       40.0 - bestPoint.dist( state.ball().pos() ) );

#ifdef DEBUG_PRINT
    dlog.addText( Logger::ACTION_CHAIN,
                  "(eval) ball pos (%f, %f)",
                  state.ball().pos().x, state.ball().pos().y );

    dlog.addText( Logger::ACTION_CHAIN,
                  "(eval) initial value (%f)", point );
#endif

    //
    // add bonus for goal, free situation near offside line
    //
    if ( FieldAnalyzer::can_shoot_from
         ( holder->unum() == state.self().unum(),
           holder->pos(),
           state.getPlayerCont( new OpponentOrUnknownPlayerPredicate( state.ourSide() ) ),
           VALID_PLAYER_THRESHOLD ) )
    {
        point += 1.0e+6;
#ifdef DEBUG_PRINT
        dlog.addText( Logger::ACTION_CHAIN,
                      "(eval) bonus for goal %f (%f)", 1.0e+6, point );
#endif

        if ( holder_unum == state.self().unum() )
        {
            point += 5.0e+5;
#ifdef DEBUG_PRINT
            dlog.addText( Logger::ACTION_CHAIN,
                          "(eval) bonus for goal self %f (%f)", 5.0e+5, point );
#endif
        }
    }

    return point;
}
