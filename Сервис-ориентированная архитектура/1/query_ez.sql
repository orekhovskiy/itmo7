create table if not exists Products(
    id bigserial primary key,
    name text,
    coordinatesX integer,
    coordinatesY bigint,
    creationDate timestamp,
    price integer,
    partNumber text,
    manufactureCost double precision,
    unitOfMeasure text,
    ownerName text,
    ownerWeight integer,
    ownerNationality text,
    ownerLocationX real,
    ownerLocationY double precision,
    ownerLocationZ double precision
);