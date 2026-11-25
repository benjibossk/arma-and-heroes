import re,json
G={}
for l in open('D:/Serveur/ww2-heroes/inventory/extract/ALL_GUIDS.txt',encoding='utf-8'):
    m=re.match(r'\{([0-9A-F]{16})\}(.*)',l.strip())
    if m: G[m.group(2)]=m.group(1)
def f(suffix):
    c=[p for p in G if p.endswith(suffix)]
    assert len(c)==1,(suffix,c)
    return '{%s}%s'%(G[c[0]],c[0])
US='Prefabs/Characters/Factions/BLUFOR/US_Army/'
GE='Prefabs/Characters/Factions/OPFOR/HEER/'
def us(n): return f(US+'USA_Infantry/Character_FH_USAWW2_Infantry_%s_Mid.et'%n)
def ge(n,fam='HEER_Infantry'): return f(GE+fam+'/Late/Character_FH_HEERWW2_%s_Late.et'%n)
C={}
def cl(k,prefab,label,cost=1,**kw):
    d={'prefab':prefab,'label':label,'cost':cost}; d.update(kw); C[k]=d
# --- US classes
cl('US_RIFLEMAN',us('Rifleman_Garand'),'US Rifleman (M1 Garand)',source='FH US Units')
cl('US_SMG',us('SMG'),'US SMG (M1A1 Thompson)',source='FH US Units')
cl('US_SL',us('SL'),'US Squad Leader',source='FH US Units')
cl('US_PL',us('PL'),'US Platoon Leader (M1 Carbine + ANM8)',source='FH US Units')
cl('US_RTO',us('RTO'),'US Radio operator (SCR-300)',source='FH US Units')
cl('US_OFFICER',us('Officer'),'US Officer (M1911)',source='FH US Units')
cl('US_SNIPER',us('Sniper'),'US Sniper (M1903A4)',cost=2,source='FH US Units')
cl('US_SR',us('SR'),'US Scout/Recon rifleman (M1 + helmet netting)',source='FH US Units')
def miss(k,label,stub,compose,cost=1):
    C[k]={'prefab':None,'status':'missing','label':label,'cost':cost,'stub_base':stub,
          'note':'Prefab de classe US vide dans FH US Units 1.0.2 (coquille heritant de BaseLoadout, sans arme). A composer dans F44_Core par override/heritage du stub.',
          'compose':compose}
miss('US_MG','US Automatic Rifleman / MG',us('MG'),{'weapon':f('Machineguns/M1918_BAR/Machinegun_M1918.et'),'alt_weapon':f('Machineguns/M1919/MG_M1919A6.et'),'hat':f('Helmet_M1_01/Helmet_M1_ww2_01.et'),'vest':f('Variants/Vest_M1936_Assembled_AutomaticRifleman.et'),'mags':'Prefabs/Weapons/Magazines/M1918_BAR/Magazine_762x63_M1918_20rnd_*'})
miss('US_MEDIC','US Medic',us('Medic'),{'weapon':f('Handguns/M1911/Handgun_M1911A1.et'),'hat':f('Helmet_M1_ww2_Medic.et'),'vest':f('Variants/Vest_M1936_Assembled_Medic.et'),'items':[f('FieldDressing_01/CarlisleFieldDressing_US_WW2.et'),'{0D9A5DCF89AE7AA9}Prefabs/Items/Medicine/MorphineInjection_01/MorphineInjection_01.et']})
miss('US_AT','US Anti-tank (Bazooka)',us('AT'),{'weapon':f('Rifles/M1_Carbine/Rifle_M1_Carbine.et'),'launcher':f('Launchers/M1A1/Launcher_M1A1_Bazooka.et'),'hat':f('Helmet_M1_01/Helmet_M1_ww2_01.et'),'extra':[f('Grenades/M7AT/Grenade_M7AT.et')]},cost=2)
miss('US_ENGINEER','US Engineer',us('Builder'),{'weapon':f('Machineguns/M1A1/Submachinegun_M1A1.et'),'items':[f('DemoBlock_TNT/DemoBlock_USA_TNT_OnePound.et'),f('Mine_M1A1/Mine_M1A1AT.et'),f('ETool_M1910/ETool_M1910.et')]})
miss('US_CREW','US Tank crew',us('Driver'),{'weapon':f('Machineguns/M3A1/Submachinegun_M3A1.et'),'hat':f('Helmet_M1938_01/Helmet_M1938_Tanker_01.et'),'jacket':f('Tanker_Jacket/Jacket_USA_Tanker.et'),'suit_alt':f('Suit_Tanker_US_01/Suit_Tanker_HBT_USA_01.et'),'handgun':f('Handguns/M1911/Handgun_M1911A1.et')})
miss('US_PARA','US Paratrooper (82nd/101st)',f(US+'Character_FH_USAWW2_BaseLoadout_Paratrooper.et'),{'weapon':f('Rifles/M1_Garand/Rifle_M1.et'),'jacket':f('M42_Jacket/Jacket_USA_M42.et'),'pants':f('M42_Trousers/Pants_USA_M42.et'),'boots':f('Footwear/JumpBoots_USA_M1942.et'),'vest':f('Vest_M1936_Assembled_Rifleman_Airborne.et'),'hat':f('Helmet_M1_01/Helmet_M1_ww2_01.et')})
# --- GER classes (Late = 1944)
cl('GER_RIFLEMAN',ge('Rifleman_KAR98_Infantry'.replace('_Infantry','')+'_Infantry') if False else f(GE+'HEER_Infantry/Late/Character_FH_HEERWW2_Rifleman_KAR98_Infantry_Late.et'),'GER Rifleman (Kar98k)',source='FH GER Units')
for k,n,l,c in [('GER_RIFLEMAN_G43','Rifleman_G43_Infantry','GER Rifleman (G43)',1),('GER_MG','MG_Infantry','GER MG gunner (MG42)',1),('GER_MEDIC','Medic_Infantry','GER Medic (Sanitater)',1),('GER_AT','AT_Infantry','GER Anti-tank (Panzerschreck RPzB54)',2),('GER_LAT','LAT_Infantry','GER Light AT (Panzerfaust 100)',2),('GER_OFFICER','Officer_Infantry','GER Officer (P38)',1),('GER_SNIPER','Sharpshooter_KAR98_Infantry','GER Sniper (Kar98k scope)',2),('GER_ENGINEER','Pioner_Infantry','GER Pioneer/Engineer (mines)',1),('GER_SMG','SMG_Infantry','GER SMG (MP40)',1),('GER_SL','SL_Infantry','GER Squad Leader (MP40)',1),('GER_PL','PL_Infantry','GER Platoon Leader (MP38)',1),('GER_RTO','RTO_Infantry','GER Radio operator',1)]:
    cl(k,f(GE+'HEER_Infantry/Late/Character_FH_HEERWW2_%s_Late.et'%n),label=l,cost=c,source='FH GER Units')
cl('GER_CREW',f(GE+'HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_PanzerCrew_Late.et'),'GER Panzer crew',source='FH GER Units')
cl('GER_PANZER_COMMANDER',f(GE+'HEER_Panzergrenadiers/Late/Character_FH_HEERWW2_PanzerCommander_Late.et'),'GER Panzer commander',source='FH GER Units')
C['GER_PARA']={'prefab':None,'status':'missing','label':'GER Fallschirmjager','cost':1,'stub_base':f(GE+'Character_FH_HEERWW2_BaseLoadoutLuftwaffe_Late.et'),'note':'Pas de classe Fallschirmjager dediee (pas de casque M38 ni smock Splittertarn). Approximation Luftwaffe.','compose':{'weapon':f('Rifles/Kar98K/Rifle_Kar98K.et') if False else 'Prefabs/Weapons/Rifles/Kar98K/Rifle_Kar98K.et (voir INVENTAIRE)','boots':f('Boots_Fallschirmjagerschuhe/Boots_HEER_JumpBoots_01.et'),'jacket':f('Blouse_M40/Jacket_HEER_M40_Luftwaffe.et'),'hat':f('Helmet_Stahlhelm_M42_Feldgrau.et') if False else 'Helmet_Stahlhelm_M42_* (GER Uniforms)'}}
# --- Vehicles
def v(k,p,label,cost,role,crew,**kw):
    C[k]={'prefab':p,'label':label,'cost':cost,'role':role,'crew':crew}; C[k].update(kw)
v('GER_KFZ',f('Kubelwagen/Kubelwagen.et'),'Volkswagen Kubelwagen',2,'light_utility','1 pilote + 3 passagers',source='FH GER Vehicles')
v('GER_KFZ_MG34',f('Kubelwagen/Kubelwagen_MG34.et'),'Kubelwagen + MG34',2,'light_armed','1 pilote + passagers + tireur',source='FH GER Vehicles')
v('GER_TRUCK',f('Opil_Bletz/Variants/Opil_Bletz_transport_covered.et'),'Opel Blitz bache (transport)',3,'truck','1 pilote + cargo',source='FH GER Vehicles')
v('GER_TRUCK_OPEN',f('Opil_Bletz/Variants/Opil_Bletz_transport.et'),'Opel Blitz plateau',3,'truck','1 pilote + cargo',source='FH GER Vehicles')
v('GER_FLAK_TRUCK',f('Opil_Bletz/Variants/Opil_Bletz_Armed_Flak38.et'),'Opel Blitz + Flak 38 (AA)',6,'light_armored_aa','1 pilote + 1-2 servants',source='FH GER Vehicles')
v('GER_SDKFZ222',f('SDKFZ_222/SDKFZ_222_Grey.et'),'Sd.Kfz. 222 (gris)',6,'light_armored','3 (pilote, tireur, commandant)',source='FH GER Vehicles',variants=[f('SDKFZ_222/SDKFZ_222_Camo.et'),f('SDKFZ_222/SDKFZ_222_Desert.et')])
v('GER_SDKFZ223',f('SDKFZ_223/SDKFZ_223_base.et'),'Sd.Kfz. 223 (radio)',6,'light_armored','3',source='FH GER Vehicles',note='racine _base : prefab placable a verifier en Workbench')
v('US_SHERMAN',f('Vehicles/Core/M4A2_Sherman.et'),'M4A2 Sherman (75 mm)',12,'tank','5 (pilote, tireur de caisse, commandant, tireur, chargeur)',source='M4A2 Sherman mod')
v('US_JEEP',  '{F649585ABB3706C4}Prefabs/Vehicles/Wheeled/M151A2/M151A2.et','Jeep M151A2 (SUBSTITUT vanilla, hors epoque)',2,'light_utility','1 pilote + 3',source='Arma Reforger vanilla',status='proxy',note='Aucune jeep WW2 US installee. Substitut visuel annees 60. Remplacer par un mod Willys.')
v('US_TRUCK', '{F1FBD0972FA5FE09}Prefabs/Vehicles/Wheeled/M923A1/M923A1_transport.et','Camion M923A1 (SUBSTITUT vanilla, hors epoque)',3,'truck','1 pilote + cargo',source='Arma Reforger vanilla',status='proxy',note='Aucun camion WW2 US (GMC CCKW) installe.')
v('GER_TIGER',None,'Tiger I (non fournissable)',12,'tank','5',status='missing',note='WW2TankCore ne contient que munitions/sons du Tiger I H ; pas de prefab vehicule.')
v('US_HALFTRACK',None,'M3 Halftrack',6,'light_armored','',status='missing',note='Absent.')
v('US_STUART_OR_ARMORED_CAR',None,'Blinde leger US (M8 Greyhound/M3 Stuart)',6,'light_armored','',status='missing',note='Absent : seul LAV-25 moderne (vanilla) existe.')
# static guns
def st(k,p,l,c): C[k]={'prefab':p,'label':l,'cost':c,'role':'static_or_towed','source':'FH GER Vehicles'}
st('GER_PAK40',f('AntiTank/Pak40/AntiTank_Pak40_Grey.et'),'Pak 40 75 mm AT (gris)',4)
st('GER_PAK36',f('AntiTank/Pak36/AntiTank_Pak36_Grey.et'),'Pak 36 37 mm AT (gris)',3)
st('GER_FLAK38',f('Artillery/AntiAir/Flak38/Autocannon_Flak38_Grey.et'),'Flak 38 20 mm (gris)',4)
st('GER_FLAKVIERLING',f('Artillery/AntiAir/Flakvierling/Autocannon_Flakvierling_Shield_Grey.et'),'Flakvierling 38 (gris)',5)
st('GER_LEFH18',f('Howitzer/LeFH18/Howitzer_LeFH18_Grey.et'),'leFH 18 obusier 105 mm (gris)',6)
out={'_meta':{'generated':'2026-11-25','note':'Cles logiques Front 44 -> prefabs Reforger. cost classes = tickets/manpower unitaire indicatif ; cost vehicules = indicatif (jeep 2, camion 3, blinde leger 6, char 12). prefab null = manquant (voir note/compose/stub_base). status=proxy : substitut hors epoque.','mods_required':['FinestHour-CoreMod','FinestHour-USGear','FinestHour-USUniforms','FinestHour-USUnits','FinestHour-GERGear','FinestHour-GERUniforms','FinestHour-GERUnits','FinestHour-GERVehicles','M4A2Sherman']},'catalog':C}
json.dump(out,open('D:/Serveur/ww2-heroes/catalog.json','w',encoding='utf-8'),indent=1,ensure_ascii=False)
print(len(C),'entries')
