#This file is distributed under the terms of the GNU General Public license.
#Copyright (C) 1999 Aloril (See the file COPYING for details).

from atlas import *
from whrandom import *
from mind.panlingua import interlinguish
il=interlinguish
from world import probability
from editor import editor
from Quaternion import Quaternion
from Vector3D import Vector3D
import time

#goal priority
#1) eating: certain times
#2) market/tavern: certain times, certain probability
#3) sleeping: nights
#4) chop trees: winter (when not doing anything else)
#4) other similar tasks: seasonal (-"-)

#'Breakfast' goal is type of 'eating'.

# Heights are all 0 for now, as uclient doesn't differentiate
# Once clients and servers can handle terrain properlly, then we
# can start thinking in more ernest about heights
settlement_height=0
forest_height=0

hall_xyz=(5,3,settlement_height)
forest_xyz=(-20,-60,settlement_height)

pig_sty_xyz=(-2,2,settlement_height)
butcher_stall_xyz=(-41.5,-6.3,settlement_height)

knowledge=[('axe','smithy'),
           ('forest',forest_xyz),
           ('hall',hall_xyz)]
mprices=[('pig','5')]
bprices=[('ham','2')]
bknowledge=[('market',butcher_stall_xyz)]
mknowledge=[('market',pig_sty_xyz)]
sknowledge=[('forest',(-30,-116,settlement_height)),
            ('stash',(-98,-97,settlement_height))]
village=[('hall', hall_xyz),
         ('butcher', butcher_stall_xyz),
         ('pig', pig_sty_xyz)]
gknowledge=[('m1',(-17, -1,    settlement_height)),
            ('m2',(-29, -1,    settlement_height)),
            ('m3',(-29, -7.5,  settlement_height)),
            ('m4',(-38, -10,   settlement_height)),
            ('m5',(-43, -15,   settlement_height)),
            ('m6',(-43, -14.5, settlement_height))]

wolf_knowledge=[('w1',(90,-90,settlement_height)),
                ('w2',(110,-90,settlement_height)),
                ('w3',(110,90,settlement_height)),
                ('w4',(90,90,settlement_height))]

lych_knowledge=[('w1',(0,-96,settlement_height)),
                ('w2',(-70,-70,settlement_height)),
                ('w3',(-100,70,settlement_height)),
                ('w4',(-147,-90,settlement_height))]

wander=(il.wander,"wander()")
forage=(il.forage,"forage()")
trade=(il.trade,"trade()")
keep=(il.keep,"keep()")
sell=(il.sell,"sell_trade()")
patrol=(il.patrol,"patrol()")

pig_goals=[(il.avoid,"avoid(['wolf','skeleton','crab'],10.0)"),
           (il.forage,"forage(self, 'acorn')"),
           (il.forage,"forage(self, 'apple')"),
           (il.herd,"herd()")]

deer_goals=[(il.avoid,"avoid(['settler','orc'],10.0)"),
            (il.herd,"herd()")]

wolf_goals=[(il.forage,"forage(self, 'ham')"),
            (il.hunt,"predate(self,'pig',30.0)"),
            (il.hunt,"predate(self,'crab',20.0)"),
            (il.patrol,"patrol(['w1', 'w2', 'w3', 'w4'])")]

crab_goals=[(il.avoid,"avoid('wolf',10.0)"),
            (il.hunt,"predate_small(self,'pig',30.0,10.0)")]

lych_goals=[(il.assemble, "assemble(self, 'skeleton', ['skull', 'ribcage', 'arm', 'pelvis', 'thigh', 'shin'])"),
            (il.patrol,"patrol(['w1', 'w2', 'w3', 'w4'])")]

directions = [[0,0,0.707,0.707],[0,0,0,1],[0,0,-0.707,0.707],[0,0,1,0],
              [0,0,0.387,0.921],[0,0,-0.387,0.921],[0,0,-0.921,0.387],[0,0,0.921,0.387]]

#observer calls this
def default(mapeditor):
#   general things

    m=editor(mapeditor)

    world=m.look()
    m.set(world.id, name="moraf")

# a wall around the world

    m.make('boundary',type='boundary',xyz=(-201,-201,settlement_height),bbox=[2,404,50])
    m.make('boundary',type='boundary',xyz=(-201,-201,settlement_height),bbox=[404,2,50])
    m.make('boundary',type='boundary',xyz=(-201, 200,settlement_height),bbox=[404,2,50])
    m.make('boundary',type='boundary',xyz=( 200,-201,settlement_height),bbox=[2,404,50])

    m.make('willow',type='willow',xyz=(-10,-0,settlement_height))
    m.make('hickory',type='hickory',xyz=(-0,-10,settlement_height))

    m.make('jetty',type='thing',xyz=(-22,-48,0),bbox=[-5, -5, -2, 5, 5, 2])
    m.make('boat',type='boat',xyz=(-22,-56,0),bbox=[-5, -2, -1, 5, 2, 1])

# a camp near the origin

    #cfire=m.make('campfire',type='campfire',xyz=(0,4,settlement_height))
    #m.make('fire',type='fire',xyz=(0.7,0.7,0),parent=cfire.id)
    #m.make('tent',type='tent',xyz=(-1,8,settlement_height),bbox=[2.5,2.5,3])
    #m.make('lumber',type='lumber',xyz=(-1,3,settlement_height))
    #m.make('lumber',type='lumber',xyz=(-1,2.5,settlement_height))

    hall=m.make('hall',type='hall',xyz=hall_xyz)

    # Fire in the centre of the hall
    cfire=m.make('campfire',type='campfire',xyz=(6,6,settlement_height),
                                            parent=hall.id)
    m.make('fire',type='fire',xyz=(0.7,0.7,0),parent=cfire.id)

    cfire=m.make('campfire',type='campfire',xyz=(3,9,settlement_height))
    m.make('fire',type='fire',xyz=(0.7,0.7,0),parent=cfire.id)

    cfire=m.make('campfire',type='campfire',xyz=(11,1,settlement_height))
    m.make('fire',type='fire',xyz=(0.7,0.7,0),parent=cfire.id)

    for i in range(0, 20):
        m.make('lumber',type='lumber',xyz=(uniform(-200,0),uniform(-200,0),settlement_height))

    m.make('weather',type='weather',desc='object that describes the weather',
           xyz=(0,1,0), rain=0.0)

#   bones all over the place
    for i in range(0, 10):
        xpos = uniform(-200,200)
        ypos = uniform(-200,200)
        m.make('skull', type='skull', xyz=(xpos+uniform(-2,2),ypos+uniform(-2,2),settlement_height))
        m.make('pelvis', type='pelvis', xyz=(xpos+uniform(-2,2),ypos+uniform(-2,2),settlement_height))
        m.make('arm', type='arm', xyz=(xpos+uniform(-2,2),ypos+uniform(-2,2),settlement_height))
        m.make('thigh', type='thigh', xyz=(xpos+uniform(-2,2),ypos+uniform(-2,2),settlement_height))
        m.make('shin', type='shin', xyz=(xpos+uniform(-2,2),ypos+uniform(-2,2),settlement_height))
        m.make('ribcage', type='ribcage', xyz=(xpos+uniform(-2,2),ypos+uniform(-2,2),settlement_height))

#   the lych, who makes bones into skeletons
    lych=m.make('lych', type='lych', xyz=(-21, -89, settlement_height))
    m.learn(lych,lych_goals)
    m.know(lych,lych_knowledge)
    m.tell_importance(lych,il.assemble,'>',il.patrol)

#   animals
    piglet = m.make('pig', type='pig', xyz=(-3,-1,settlement_height))
    m.learn(piglet,pig_goals)

    wolf = m.make('wolf', type='wolf', xyz=(90,-90,settlement_height))
    m.learn(wolf,wolf_goals)
    m.know(wolf,wolf_knowledge)
    m.tell_importance(wolf,il.forage,'>',il.hunt)
    m.tell_importance(wolf,il.forage,'>',il.patrol)
    m.tell_importance(wolf,il.hunt,'>',il.patrol)

    crab = m.make('crab', type='crab', xyz=(-90,90,settlement_height))
    m.learn(crab,crab_goals)

    skeleton = m.make('skeleton', type='skeleton', xyz=(-38,-25,settlement_height))

    #squirrel = m.make('squirrel', type='squirrel', desc='test squirrel',
                    #xyz=(-32,-115,settlement_height))
    #m.know(squirrel,sknowledge)
    #m.learn(squirrel,(il.transport,"transport_something(self,'acorn','forest','stash')"))

#   villagers
    #directions = [[0,1,0],[1,0,0],[0,-1,0],[-1,0,0],
                  #[0.7,0.7,0],[0.7,-0.7,0],[-0.7,-0.7,0],[-0.7,0.7,0]]

    home1_xyz=(90,-90,settlement_height)

    butcher=m.make('Ulad Bargan',type='butcher',desc='the butcher',
                 xyz=butcher_stall_xyz,age=probability.fertility_age,sex='male')
    m.learn(butcher,(il.trade,"trade(self, 'pig', 'cleaver', 'cut', 'ham', 'market','day')"))
    m.learn(butcher,(il.buy_livestock,"buy_livestock('pig', 1)"))
    m.learn(butcher,(il.market,"run_shop('mstall_freshmeat_1_se','open','dawn')"))
    m.learn(butcher,(il.market,"run_shop('mstall_freshmeat_1_se','closed','evening')"))
    m.know(butcher,bknowledge)
    m.price(butcher,bprices)
    cleaver=m.make('cleaver', type='cleaver', desc='cleaver for cutting meat',
                   place='market', xyz=(-41,-5,settlement_height))
    m.own(butcher,cleaver)
    m.learn(butcher,(il.sell,"sell_trade('ham', 'market')"))
    coins=[]
    for i in range(0, 60):
        coins.append(m.make('coin',type='coin',xyz=(0,0,0),parent=butcher.id))
    m.own(butcher,coins)
    

    home2_xyz=(80,80,settlement_height)
    merchant=m.make('Dyfed Searae',type='merchant',desc='the pig merchant',
                    xyz=pig_sty_xyz,age=probability.fertility_age,
                    sex='male',orientation=Quaternion(Vector3D([1,0,0]),Vector3D([-1,0,0])).as_list())
    sty=m.make('sty',type='sty',xyz=pig_sty_xyz,status=1.0,bbox=[5,5,3])
    m.know(merchant,mknowledge)
    m.know(merchant,village)
    m.price(merchant,mprices)
    m.own(merchant,sty)
    m.learn(merchant,(il.keep,"keep_livestock('pig', 'sty', 'sowee')"))
    m.learn(merchant, (il.sell,"sell_trade('pig', 'market', 'morning')"))
    m.learn(merchant, (il.sell,"sell_trade('pig', 'market', 'afternoon')"))
    m.learn(merchant,(il.lunch,"meal(self, 'ham','midday', 'inn')"))
    m.learn(merchant,(il.sup,"meal(self, 'beer', 'evening', 'inn')"))
    m.learn(merchant,(il.welcome,"welcome('Welcome to this our settlement','settler')"))
    m.learn(merchant,(il.help,"add_help(['Thankyou for joining our remote settlement.','Our first task is to build some shelter, but while we are doing that we still need food.','You can help us out by raising pigs for slaughter.','If you want to buy a piglet to raise, let me know by saying you would like to buy one.','Pigs love to eat acorns from under the oak trees that are abundant in this area.'])"))
    piglets=[]
    for i in range(0, 6):
        piglets.append(m.make('pig',type='pig',xyz=(uniform(0,4),uniform(0,4),settlement_height),parent=sty.id,orientation=directions[randint(0,7)]))
    m.learn(piglets,pig_goals)
    m.own(merchant,piglets)

    # Warriors - the more adventurous types

    warriors=[]
    warrior=m.make('Vonaa Barile',type='settler',xyz=(uniform(-1,14),uniform(-18,-27),settlement_height),sex='female',orientation=directions[randint(0,7)])
    m.learn(warrior,(il.hunt,"hunt(self, 'bow', 'deer', 10)"))
    bow=m.make('bow',type='bow',xyz=(0,0,0), parent=warrior.id)
    m.own(warrior,bow)
    warriors.append(warrior)

    warrior=m.make('Lile Birloc', type='settler',xyz=(-2,-2,settlement_height),sex='female',orientation=directions[randint(0,7)])
    m.learn(warrior,(il.hunt,"hunt(self, 'bow', 'deer', 10)"))
    bow=m.make('bow',type='bow',xyz=(0,0,0), parent=warrior.id)
    m.own(warrior,bow)
    for i in range(0, 6):
        arrow=m.make('arrow',type='arrow',xyz=(0,0,0), parent=warrior.id)
        m.own(warrior,arrow)
    warriors.append(warrior)

    # Warriors all know where stuff is in the village
    m.know(warriors,village)

    # Warriors enjoy their food and drink
    m.learn(warriors,(il.lunch,"meal(self, 'ham','midday', 'inn')"))
    m.learn(warriors,(il.sup,"meal(self, 'beer', 'evening', 'inn')"))

    deers=[]
    xbase = uniform(-180,180)
    ybase = uniform(-180,180)
    for i in range(0, 10):
        xpos = xbase + uniform(-20,20)
        ypos = ybase + uniform(-20,20)
        d=m.make('deer', type='deer', xyz=(xpos, ypos, settlement_height))
        deers.append(d)
    m.learn(deers,deer_goals)
    

    # I am not sure if we need a guard
    #m.learn(guard,(il.patrol,"patrol(['m1', 'm2', 'm3', 'm4', 'm5', 'm6'])"))
    #m.tell_importance(guard,il.defend,'>',il.patrol)

def add_pigs(mapeditor):
#   general things

    m=editor(mapeditor)

    sty = m.look_for(type='sty')
    merchant = m.look_for(name='Dyfed Searae')

    piglets=[]
    for i in range(0, 6):
        piglets.append(m.make('pig',type='pig',xyz=(uniform(0,4),uniform(0,4),settlement_height),parent=sty.id,orientation=directions[randint(0,7)]))
    m.learn(piglets,pig_goals)
    m.own(merchant,piglets)
