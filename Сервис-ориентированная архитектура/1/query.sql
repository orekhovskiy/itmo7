create table if not exists Country (
    id integer primary key,
    name varchar(20)
);

insert into Country values (0, 'USA') ON CONFLICT DO NOTHING;
insert into Country values (1, 'CHINA') ON CONFLICT DO NOTHING;
insert into Country values (2, 'INDIA') ON CONFLICT DO NOTHING;
insert into Country values (3, 'THAILAND') ON CONFLICT DO NOTHING;

create table if not exists UnitOfMeasure (
    id integer primary key,
    name varchar(20)
);

insert into UnitOfMeasure values (0, 'CENTIMETERS') ON CONFLICT DO NOTHING;
insert into UnitOfMeasure values (1, 'GRAMS') ON CONFLICT DO NOTHING;
insert into UnitOfMeasure values (2, 'MILLIGRAMS') ON CONFLICT DO NOTHING;

create table if not exists Location (
    id serial primary key,
    x real,
    y double precision not null,
    z double precision
);

create table if not exists Coordinates (
    id serial primary key,
    x integer check (x > -965),
    y bigint
);

create table if not exists Person (
    id serial primary key,
    name text not null check (name <> ''),
    weight integer null check (weight > 0),
    nationalityId integer references Country(id) not null,
    locationId integer references Location(id) not null
);

create table if not exists Product (
    id bigserial primary key,
    name text not null check (name <> ''),
    coordinatesId integer references Coordinates(id) not null,
    creationDate timestamp not null,
    price integer null check (price > 0),
    partNumber text null unique check (length(partNumber) >= 27),
    manufactureCost double precision,
    unitOfMeasureId integer references UnitOfMeasure(id) null,
    ownerId integer references Person(id) not null
);