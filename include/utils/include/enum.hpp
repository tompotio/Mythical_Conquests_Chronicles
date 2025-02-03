#pragma once

enum EWorld {
    Lobby,
    Test,
};

enum ELook {
    Up,
    UpRight,
    UpLeft,
    Down,
    DownRight,
    DownLeft,
    Right,
    Left
};

enum EntityState {
    IdleState, 
    WalkingState,
    RunningState,
    HSlashState,
    VSlashState,
    RunPlusAttackState,
    SHOTSTATE,
};

enum PClass {
    MageClass,
    FighterClass,
};

enum PSkill {
    HSlashSkill,
};

enum ETypeInstance {
    PLAYERTYPE,
    ENEMYTYPE,
    TILEDYNAMIQUE,
    TILEWALL,
    TILEWATER,
    TILESTATIQUE,
    TARGETTYPE,
    KNOCKBACKTYPE,
    PROJECTILETYPE,
    RUNPLUSATTACK,
};

enum BodyType {
    Static,
    Kinematic,
    Dynamic,
};

enum NetworkCommands {
    Position, // Position du joueur.
    Speed, // Vitesse du joueur.
    Message, // Message...
    NewPlayer, // Un nouveau joueur a rejoint.
    GiveYouId, // Notifie notre nouvel id.
    State, // L'état d'un joueur.
    PlayerLeft, // Un joueur a quitté.
};

enum GameState {
    InGame,
    InMenu,
    InLoadingScreen,
};

enum groupIndex {
    PLAYERINDEX = -1,
    ENEMYINDEX = -3,
    PROJECTILEINDEX = -3,
    TILEINDEX = 4,
    BORDERINDEX = 5,
};

enum NuklearImage {
    BulbHover,
    Bulb,
    BulbClick,
};