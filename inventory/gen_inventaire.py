import re,os,json,glob
R='D:/Serveur/ww2-heroes/inventory/extract/'
ADD='D:/Serveur/arma-reforger-ww2/profile/addons/'
G={}
for l in open(R+'ALL_GUIDS.txt',encoding='utf-8'):
    m=re.match(r'\{([0-9A-F]{16})\}(.*)',l.strip())
    if m: G[m.group(2)]=m.group(1)
def per_mod(mod):
    d={}
    for l in open(R+mod+'.guids.txt',encoding='utf-8'):
        m=re.match(r'\{([0-9A-F]{16})\}(.*)',l.strip())
        if m: d[m.group(2)]=m.group(1)
    return d
def rs(guid,p): return '{%s}%s'%(guid,p)
def txt(mod,p):
    f=R+mod+'/'+p
    return open(f,encoding='utf-8',errors='replace').read() if os.path.exists(f) else ''
mods=[]
for d in sorted(os.listdir(ADD)):
    if d=='saves': continue
    short,gid=d.rsplit('_',1)
    sd=json.load(open(ADD+d+'/ServerData.json',encoding='utf-8-sig'))
    mods.append((short,gid,sd['name'],sd['revision']['version'],os.path.getsize(ADD+d+'/data.pak')//1048576))
out=[]
w=out.append
w('# INVENTAIRE ASSETS WW2 - Front 44\n')
w('Genere le 2026-11-25 depuis les .pak de `D:\\Serveur\\arma-reforger-ww2\\profile\\addons` (lecture seule). Listings et textes extraits dans `inventory/extract/` (`<mod>.listing.txt` = tout le pak, `<mod>.guids.txt` = `{GUID}chemin` des .et/.conf lus dans resourceDatabase.rdb, `ALL_GUIDS.txt` = fusion). Les resource names ci-dessous sont complets et utilisables tels quels.\n')
w('## Mods\n\n| Mod | GUID addon | Version | Pak (Mo) |\n|---|---|---|---|')
for s,g,n,v,sz in mods: w(f'| {n} (`{s}`) | `{g}` | {v} | {sz} |')
w('')
w('> Alerte : **Finest Hour - US Units 1.0.2 est largement un squelette** (pak 39 Ko) : seules 8 classes Infantry ont un equipement (Officer, PL, Rifleman Garand, RTO, SL, SMG, SR, Sniper). MG/AT/Medic/Crew/etc. US = coquilles vides heritant de BaseLoadout. `Group_FH_USAWW2_RifleSquad` reference `Character_FH_USAWW2_SL/Medic/Rifleman.et` qui n\'existent dans aucun mod installe. Le catalogue `Characters_EntityCatalog_USA_WW2.conf` est vide. Idem Vehicles catalogs USA/HEER (listes de categories vides).\n')

# ---------- Factions
w('## 1. Factions\n')
cm=per_mod('FinestHour-CoreMod')
w('| FactionKey | Fichier | Remarque |\n|---|---|---|')
for fk,p,rm in [('FH_USAWW2','Configs/Factions/USA_WW2.conf','herite de BLUFOR.conf ; amis : US, CIV'),('FH_HEERWW2','Configs/Factions/HEER_WW2.conf','herite de OPFOR.conf'),('FH_USSRWW2','Configs/Factions/USSR_WW2.conf','NE PAS UTILISER : catalogues = unites USSR vanilla modernes (Spetsnaz, UAZ, BTR-70, Mi-8), pas WW2')]:
    w(f'| `{fk}` | `{rs(cm[p],p)}` | {rm} |')
w('\nCatalogues d\'entites (CoreMod) : '+', '.join(f'`{rs(cm[p],p)}`' for p in sorted(cm) if p.startswith('Configs/EntityCatalog/') and ('Groups' in p or 'Characters' in p or 'Vehicles' in p) and 'USSR' not in p)+'\n')
w('Autres configs : Callsigns_HEER, FactionHomeTerritory_HEER, FieldManual categories (CoreMod/GERVehicles).\n')

# ---------- Characters
def chars(mod,prefix,label):
    d=per_mod(mod)
    rows=[]
    for p,g in sorted(d.items()):
        if not p.startswith('Prefabs/Characters/Factions/') or not p.endswith('.et'): continue
        t=txt(mod,p)
        wt=[os.path.basename(x) for x in re.findall(r'WeaponTemplate "\{[0-9A-F]+\}([^"]+)"',t)]
        sl=dict(re.findall(r'LoadoutSlotInfo (\w+) \{\s*Prefab "\{[0-9A-F]+\}([^"]+)"',t))
        stub = len(t)<200
        rows.append((p,g,wt,sl,stub))
    return rows
w('## 2. Personnages / loadouts jouables\n')
w('### 2.1 Finest Hour - US Units (`FH_USAWW2`, BLUFOR)\n')
w('Mid-war uniquement (casque M1, Garand). Hierarchie : Base -> BaseLoadout (+_Late, _Paratrooper = coquilles) -> classes `USA_Infantry` / `USA_ArmoredInfantry` / `USA_CavRecon` (suffixe `_Mid`). STUB = prefab vide (149 octets, aucun equipement).\n')
w('| Prefab | Arme(s) | Casque / Gilet | Etat |\n|---|---|---|---|')
for p,g,wt,sl,stub in chars('FinestHour-USUnits','',''):
    sh=p.replace('Prefabs/Characters/Factions/BLUFOR/US_Army/','')
    w(f'| `{rs(g,p)}` | {", ".join(wt) or "-"} | {os.path.basename(sl.get("Hat","-"))} / {os.path.basename(sl.get("Vest","-"))} | {"STUB vide" if stub else "OK"} |')
w('\nGroupes : '+', '.join(f'`{rs(g,p)}`' for p,g in sorted(per_mod('FinestHour-USUnits').items()) if p.startswith('Prefabs/Groups'))+' (RifleSquad casse : references manquantes).\n')
w('### 2.2 Finest Hour - GER Units (`FH_HEERWW2`, OPFOR)\n')
w('Mid et Late (Late = 1944-45 : Stahlhelm M42, Kar98k/G43/MP40/MG42, Panzerfaust/Panzerschreck). Sous-familles : HEER_Infantry, HEER_Panzergrenadiers, HEER_Aufklarers, Luftwaffe. Tableau = versions Late + PanzerCrew + Luftwaffe.\n')
w('| Prefab | Arme(s) | Casque / Gilet |\n|---|---|---|')
for p,g,wt,sl,stub in chars('FinestHour-GERUnits','',''):
    if not (re.search(r'Late',p) or 'PanzerCrew' in p or 'PanzerCommander' in p or 'PanzerOfficer' in p or 'Luftwaffe' in p or 'Base' in os.path.basename(p)): continue
    w(f'| `{rs(g,p)}` | {", ".join(wt) or "-"} | {os.path.basename(sl.get("Hat","-"))} / {os.path.basename(sl.get("Vest","-"))} |')
n=len([1 for p in per_mod('FinestHour-GERUnits') if p.startswith('Prefabs/Characters')])
w(f'\nTotal {n} prefabs personnages GER (cf. `FinestHour-GERUnits.listing.txt` pour les variantes Mid). Groupes GER ({len([1 for p in per_mod("FinestHour-GERUnits") if p.startswith("Prefabs/Groups")])}) : voir `FinestHour-GERUnits.guids.txt` (`Prefabs/Groups/OPFOR/HEER_*`).\n')

# ---------- Weapons
def mag(mod,p):
    t=txt(mod,p)
    m=re.search(r'MagazineTemplate "\{[0-9A-F]+\}([^"]+)"',t)
    if m: return os.path.basename(m.group(1)).replace('.et','')
    par=re.match(r'\S+ : "\{[0-9A-F]+\}([^"]+)"',t)
    if par:
        pp=par.group(1)
        for mm in (mod,):
            t2=txt(mm,pp)
            m=re.search(r'MagazineTemplate "\{[0-9A-F]+\}([^"]+)"',t2)
            if m: return os.path.basename(m.group(1)).replace('.et','')
    return '-'
w('## 3. Armes (armes d\'epaule, pistolets, MG, lanceurs)\n')
wcat=r'^Prefabs/Weapons/(Rifles|Handguns|Machineguns|MachineGuns|Submachineguns|Launchers)/'
for mod in ['FinestHour-USGear','FinestHour-GERGear','ChungusBoltGuns','FinestHour-CoreMod','FinestHour-GERVehicles']:
    d=per_mod(mod)
    rows=[(p,g) for p,g in sorted(d.items()) if re.match(wcat,p) and p.endswith('.et') and not re.search(r'Attachments|_base|_Base|/Part|Part_|Tripod',p)]
    if not rows: continue
    w(f'### {mod}\n\n| Arme | Resource name | Magasin par defaut (-> calibre) |\n|---|---|---|')
    for p,g in rows: w(f'| {os.path.basename(p)[:-3]} | `{rs(g,p)}` | {mag(mod,p)} |')
    w('')
w('Tripodes/affuts : '+', '.join(f'`{rs(g,p)}`' for mod in ['FinestHour-USGear','FinestHour-GERGear'] for p,g in sorted(per_mod(mod).items()) if 'Tripods/' in p and p.endswith('.et') and not re.search(r'Part|base|Base',p))+'\n')
w('WCS_Armaments / WCS_SpaceCore : armes **modernes** (AK, M16, lance-roquettes, missiles) - hors sujet WW2, non detaillees (voir `WCS_Armaments.guids.txt`).\n')

# ---------- Munitions / explosifs
w('## 4. Munitions, grenades, explosifs\n')
for mod in ['FinestHour-USGear','FinestHour-GERGear']:
    d=per_mod(mod)
    w(f'### {mod}\n')
    for lab,rx in [('Grenades',r'^Prefabs/Weapons/Grenades/'),('Explosifs / mines',r'^Prefabs/Weapons/Explosives/'),('Lanceurs (munitions roquette/rifle grenade)',r'^Prefabs/Weapons/Ammo/(Rifle_Grenades|No82)')]:
        rows=[(p,g) for p,g in sorted(d.items()) if re.match(rx,p) and p.endswith('.et') and not re.search(r'Submunition|_base|_Base|EffectModule',p)]
        if rows: w(f'**{lab}** : '+'; '.join(f'`{rs(g,p)}`' for p,g in rows)+'\n')
    mg=[(p,g) for p,g in sorted(d.items()) if p.startswith('Prefabs/Weapons/Magazines/') and p.endswith('.et')]
    w(f'**Magasins ({len(mg)})** : voir `{mod}.guids.txt` (`Prefabs/Weapons/Magazines/*`).\n')
w('CoreMod : munitions (`Prefabs/Weapons/Ammo`, 122 fichiers), obus (`Configs/Weapons/Ammo/ShellConf`), tables balistiques mortier, warheads : cf. `FinestHour-CoreMod.guids.txt`.\n')

# ---------- Uniformes
w('## 5. Tenues, casques, gilets, equipements\n')
for mod in ['FinestHour-USUniforms','FinestHour-GERUniforms']:
    d=per_mod(mod)
    w(f'### {mod}\n')
    for lab,rx in [('Casques / couvre-chefs',r'^Prefabs/Characters/HeadGear/'),('Vestes / pantalons / combinaisons',r'^Prefabs/Characters/Uniforms/'),('Gilets',r'^Prefabs/Characters/Vests/'),('Chaussures / gants',r'^Prefabs/Characters/(Footwear|Handwear)/'),('Sacs / radios / accessoires',r'^Prefabs/Items/Equipment/')]:
        rows=[(p,g) for p,g in sorted(d.items()) if re.match(rx,p) and p.endswith('.et') and not re.search(r'Variants|Assembled|/Part',p)]
        w(f'**{lab} ({len(rows)})**\n')
        for p,g in rows[:60]: w(f'- `{rs(g,p)}`')
        if len(rows)>60: w(f'- ... +{len(rows)-60} (voir guids.txt)')
        w('')
    pc=[(p,g) for p,g in sorted(d.items()) if 'Vests' in p and 'Assembled' in p and p.endswith('.et')]
    w(f'Gilets assembles (classes) : {len(pc)} - utilises par les loadouts (`Vest_M1936_Assembled_*`, `Airborne/*` pour paras) ; voir `{mod}.guids.txt`.\n')
for mod in ['FinestHour-USGear','FinestHour-GERGear']:
    d=per_mod(mod)
    rows=[(p,g) for p,g in sorted(d.items()) if p.startswith('Prefabs/Items/') and p.endswith('.et')]
    w(f'### {mod} - objets (Items) : '+'; '.join(f'`{rs(g,p)}`' for p,g in rows[:40])+('; ...' if len(rows)>40 else '')+'\n')

# ---------- Vehicles
def slots(mod,p):
    t=txt(mod,p)
    return (t.count('PilotCompartmentSlot '),t.count('CargoCompartmentSlot '),t.count('TurretCompartmentSlot '))
w('## 6. Vehicules\n')
w('Roles/equipage : Pilote/Cargo/Tourelle = nombre de `CompartmentSlot` dans le .et racine ; les sieges ajoutes par les VehParts (prefabs `VehParts/Seats`) augmentent le total reel.\n')
w('| Vehicule | Resource name | Role | Slots P/C/T |\n|---|---|---|---|')
veh=[('FinestHour-GERVehicles',p) for p in per_mod('FinestHour-GERVehicles') if re.match(r'Prefabs/Vehicles/Wheeled/(Kubelwagen/(Kubelwagen|Kubelwagen_MG34|Kubelwagen_Canvas)|Opil_Bletz/(Variants/.*)|SDKFZ_22[23]/SDKFZ_22[23]_.*|SdAh51_Trailer/Trailer_SdAh51_transport)\.et$',p)]
veh+= [('M4A2Sherman','Prefabs/Vehicles/Core/M4A2_Sherman.et')]
roles={'Kubelwagen.et':'Voiture legere 4x4 (jeep allemande)','Kubelwagen_MG34.et':'Voiture legere + MG34','Kubelwagen_Canvas.et':'Voiture legere bachee','SDKFZ_222':'Voiture blindee legere (canon 20 mm KwK38 + MG)','SDKFZ_223':'Voiture blindee radio','Opil_Bletz':'Camion Opel Blitz','M4A2_Sherman':'Char moyen M4A2 Sherman (75 mm M3)','Trailer':'Remorque'}
def role(p):
    for k,v in roles.items():
        if k in p: return v
    return ''
for mod,p in sorted(veh,key=lambda x:x[1]):
    if 'SDKFZ_223_Turret' in p: continue
    w(f'| {os.path.basename(p)[:-3]} | `{rs(per_mod(mod)[p],p)}` | {role(p)} | {"/".join(map(str,slots(mod,p)))} |')
w('')
w('`0/0/0` = slots herites du prefab parent (variantes). Equipage reel : Kubelwagen 1 pilote + passagers ; SdKfz 222 pilote + tourelle (2-3 membres) ; Opel Blitz 1 pilote + cargo ; Sherman 1 pilote + 5 cargo (sieges equipage via tourelle, Weapons/Core/M4A2_Turret_base).')
w('**Artillerie / pieces tractees et fixes** (voir section 7).\n')
w('**WW2TankCore** : bibliotheque (ammo Tiger I H, sons Pershing/Tiger, parties de BRDM2/UAZ) - **ne contient PAS de prefab de char jouable** (le Tiger I n\'a que ses 3 magasins `vehparts` et ses munitions `Prefabs/Weapons/Ammo/TigerH1/*`).\n')
w('**CIE Tracked Core / CIE Thermal / Chungus Core** : bibliotheques techniques (`Vehicles/Tracked/LTM_Tracked_Vehicles_Base.et`, thermique, base chargeurs) ; aucun vehicule WW2 directement placable.\n')
w('**WCS_Armaments** : vehicules modernes (UH-1H, Mi-8MT, BTR-70, UAZ, Ural, M998/M1025, S105 rally...) - hors periode. Reperes utiles hors mods : vehicules **vanilla Reforger** M151A2 (`{F649585ABB3706C4}Prefabs/Vehicles/Wheeled/M151A2/M151A2.et`) et M923A1 (`{F1FBD0972FA5FE09}Prefabs/Vehicles/Wheeled/M923A1/M923A1_transport.et`) = substituts visuels US (annees 60-80).\n')

# ---------- Artillery
w('## 7. Artillerie, AA, AT, canons, mortiers\n')
def lst(mod,rx,ex=r'GunMount|Part|Destroyed'):
    d=per_mod(mod); return [(p,g) for p,g in sorted(d.items()) if re.search(rx,p) and p.endswith('.et') and not re.search(ex,p)]
for lab,mod,rx in [('GERVehicles - AT/AA/obusiers (Grey = defaut 1944)','FinestHour-GERVehicles',r'^Prefabs/Weapons/Artillery/.*(Grey)'),('GERVehicles - mortiers','FinestHour-GERVehicles',r'^Prefabs/Weapons/Mortars/'),('CoreMod - canons US/GER (HeavyWeapons)','FinestHour-CoreMod',r'^Prefabs/Weapons/HeavyWeapons/'),('USGear - lanceurs','FinestHour-USGear',r'^Prefabs/Weapons/Launchers/'),('GERGear - lanceurs','FinestHour-GERGear',r'^Prefabs/Weapons/Launchers/')]:
    rows=lst(mod,rx)
    w(f'**{lab}** :')
    for p,g in rows: w(f'- `{rs(g,p)}`')
    w('')
w('Fire missions : `PrefabsEditable/EffectsModules/Mortar/*` (barrages zones, WP) dans CoreMod ; tables balistiques `Configs/Weapons/Ammo/MortarBallisticTableGeneratorConfigs`.\n')

# ---------- Autres
w('## 8. Autres\n')
d=per_mod('FinestHour-CoreMod')
arsenals=[(p,g) for p,g in sorted(d.items()) if 'Arsenal' in p and p.endswith('.et') and p.startswith('Prefabs/Props/Military/Arsenal')]
w(f'**Arsenaux / caisses ({len(arsenals)})** : '+'; '.join(f'`{rs(g,p)}`' for p,g in arsenals[:12])+' ...\n')
svc=[(p,g) for p,g in sorted(per_mod('FinestHour-USGear').items()) if p.endswith('.et') and re.search(r'Service|Spawn|Radio',p)]
w('**Services / spawn US** : '+'; '.join(f'`{rs(g,p)}`' for p,g in svc)+'\n')
w('**Northcom German Voice Pack** : sons/voix allemandes uniquement (aucun .et/.conf ; 140 fichiers audio) - cf. `NorthcomGermanVoicePack.listing.txt`.\n')
w('**Radio / protocole** : `Sounds/RadioProtocol/RadioProtocol_Report_2_US.acp` (US) ; voix GER via Northcom.\n')
open('D:/Serveur/ww2-heroes/inventory/INVENTAIRE.md','w',encoding='utf-8').write('\n'.join(out))
print(len('\n'.join(out)))
