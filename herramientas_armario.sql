create database armario;
show databases;
use armario;
create TABLE herramientas(
	id int,
    herramienta varchar(255),
    estado bool,
    PRIMARY KEY (id)
);

-- INSERT INTO Herramientas (herramienta, estado) VALUES ('soldador',1);
-- esta linea no sera ejecutada

ALTER TABLE herramientas MODIFY COLUMN id int auto_increment;
SHOW CREATE TABLE herramientas;

CREATE TABLE `herramientas` (
  `id` int NOT NULL AUTO_INCREMENT,
  `herramienta` varchar(255) DEFAULT NULL,
  `estado` tinyint(1) DEFAULT NULL,
  PRIMARY KEY (`id`)
);

INSERT INTO Herramientas (herramienta, estado) VALUES ('soldador',1);
INSERT INTO Herramientas (herramienta, estado) VALUES ('desoldador',1);
INSERT INTO Herramientas (herramienta, estado) VALUES ('estaño',1);
   
SELECT * FROM herramientas;
SELECT * FROM herramientas WHERE id = 1;
SELECT * FROM herramientas WHERE estado = 1;
SELECT * FROM herramientas WHERE estado = 1 AND herramienta = 'estaño';

UPDATE herramientas SET estado = 0 where id = 3;
SELECT * FROM herramientas;
delete  from herramientas where id = 1;
delete  from herramientas where id = 5;
INSERT INTO Herramientas (id,herramienta, estado) VALUES (1,'soldador',1);
UPDATE herramientas SET herramienta = 'fuente de alimentacion' where id = 1;

create TABLE datos(
	id int NOT NULL AUTO_INCREMENT,
    Profesor varchar(255),
    Herramientas varchar(255),
    Evento bool,
    PRIMARY KEY (id)
);

SELECT * FROM datos;

ALTER TABLE datos auto_increment=1; 