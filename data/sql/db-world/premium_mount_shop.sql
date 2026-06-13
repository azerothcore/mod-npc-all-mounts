-- =============================================================================
-- Premium Mount Shop - tabla de monturas vendibles
-- =============================================================================
-- El worldserver también crea esta tabla automáticamente al arrancar si no existe.
-- Este archivo sirve para instalación manual y datos de ejemplo.
-- =============================================================================

DROP TABLE IF EXISTS `premium_mount_shop`;
CREATE TABLE `premium_mount_shop` (
    `id` INT UNSIGNED AUTO_INCREMENT PRIMARY KEY,
    `category` VARCHAR(50) NOT NULL DEFAULT 'General',
    `spellid` INT UNSIGNED NOT NULL,
    `name` VARCHAR(100) NOT NULL,
    `premium_cost` INT UNSIGNED NOT NULL DEFAULT 1,
    `enabled` TINYINT(1) NOT NULL DEFAULT 1,
    UNIQUE KEY (`spellid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- =============================================================================
-- Datos de ejemplo (categorías cargadas dinámicamente desde la columna category)
-- premium_cost = precio base para cuentas premium; no premium paga cost * multiplier
-- =============================================================================

-- Monturas Terrestres
INSERT INTO `premium_mount_shop` (`category`, `spellid`, `name`, `premium_cost`, `enabled`) VALUES
('Monturas Terrestres', 470, 'Black Stallion', 1, 1),
('Monturas Terrestres', 458, 'Brown Horse', 1, 1),
('Monturas Terrestres', 6648, 'Chestnut Mare', 1, 1),
('Monturas Terrestres', 472, 'Pinto', 1, 1),
('Monturas Terrestres', 580, 'Timber Wolf', 1, 1),
('Monturas Terrestres', 6653, 'Dire Wolf', 1, 1),
('Monturas Terrestres', 6898, 'White Ram', 1, 1),
('Monturas Terrestres', 6899, 'Brown Ram', 1, 1),
('Monturas Terrestres', 17463, 'Blue Skeletal Horse', 1, 1),
('Monturas Terrestres', 64659, 'Venomhide Ravasaur', 3, 1);

-- Monturas Voladoras
INSERT INTO `premium_mount_shop` (`category`, `spellid`, `name`, `premium_cost`, `enabled`) VALUES
('Monturas Voladoras', 32235, 'Golden Gryphon', 2, 1),
('Monturas Voladoras', 32239, 'Ebon Gryphon', 2, 1),
('Monturas Voladoras', 32240, 'Snowy Gryphon', 2, 1),
('Monturas Voladoras', 32244, 'Blue Wind Rider', 2, 1),
('Monturas Voladoras', 32245, 'Green Wind Rider', 2, 1),
('Monturas Voladoras', 40192, 'Ashes of Al''ar', 10, 1),
('Monturas Voladoras', 60025, 'Albino Drake', 3, 1),
('Monturas Voladoras', 59567, 'Azure Drake', 4, 1),
('Monturas Voladoras', 61294, 'Green Proto-Drake', 5, 1),
('Monturas Voladoras', 63956, 'Ironbound Proto-Drake', 6, 1);

-- Monturas JcJ
INSERT INTO `premium_mount_shop` (`category`, `spellid`, `name`, `premium_cost`, `enabled`) VALUES
('Monturas JcJ', 22717, 'Black War Steed', 3, 1),
('Monturas JcJ', 22723, 'Black War Tiger', 3, 1),
('Monturas JcJ', 22724, 'Black War Wolf', 3, 1),
('Monturas JcJ', 22720, 'Black War Ram', 3, 1),
('Monturas JcJ', 22721, 'Black War Raptor', 3, 1),
('Monturas JcJ', 22718, 'Black War Kodo', 3, 1),
('Monturas JcJ', 23509, 'Frostwolf Howler', 4, 1),
('Monturas JcJ', 23510, 'Stormpike Battle Charger', 4, 1),
('Monturas JcJ', 64927, 'Deadly Gladiator''s Frost Wyrm', 8, 1),
('Monturas JcJ', 71810, 'Wrathful Gladiator''s Frost Wyrm', 10, 1);

-- Monturas Raras
INSERT INTO `premium_mount_shop` (`category`, `spellid`, `name`, `premium_cost`, `enabled`) VALUES
('Monturas Raras', 72286, 'Invincible', 5, 1),
('Monturas Raras', 17481, 'Rivendare''s Deathcharger', 5, 1),
('Monturas Raras', 36702, 'Fiery Warhorse', 5, 1),
('Monturas Raras', 41252, 'Raven Lord', 4, 1),
('Monturas Raras', 42776, 'Spectral Tiger', 15, 1),
('Monturas Raras', 42777, 'Swift Spectral Tiger', 15, 1),
('Monturas Raras', 63796, 'Mimiron''s Head', 8, 1),
('Monturas Raras', 69395, 'Onyxian Drake', 6, 1),
('Monturas Raras', 60002, 'Time-Lost Proto-Drake', 7, 1),
('Monturas Raras', 71342, 'Big Love Rocket', 10, 1);

-- Monturas Personalizadas
INSERT INTO `premium_mount_shop` (`category`, `spellid`, `name`, `premium_cost`, `enabled`) VALUES
('Monturas Personalizadas', 75614, 'Celestial Steed', 3, 1),
('Monturas Personalizadas', 75973, 'X-53 Touring Rocket', 4, 1),
('Monturas Personalizadas', 74856, 'Blazing Hippogryph', 4, 1),
('Monturas Personalizadas', 73313, 'Crimson Deathcharger', 5, 1),
('Monturas Personalizadas', 65917, 'Magic Rooster', 2, 1);
