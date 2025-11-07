/*M!999999\- enable the sandbox mode */ 
-- MariaDB dump 10.19-12.0.2-MariaDB, for Linux (x86_64)
--
-- Host: localhost    Database: web_pkl
-- ------------------------------------------------------
-- Server version	12.0.2-MariaDB

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*M!100616 SET @OLD_NOTE_VERBOSITY=@@NOTE_VERBOSITY, NOTE_VERBOSITY=0 */;

--
-- Table structure for table `Jurusan`
--

DROP TABLE IF EXISTS `Jurusan`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `Jurusan` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `Nama` varchar(100) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Jurusan`
--

LOCK TABLES `Jurusan` WRITE;
/*!40000 ALTER TABLE `Jurusan` DISABLE KEYS */;
set autocommit=0;
/*!40000 ALTER TABLE `Jurusan` ENABLE KEYS */;
UNLOCK TABLES;
commit;

--
-- Table structure for table `Kelas`
--

DROP TABLE IF EXISTS `Kelas`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `Kelas` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `Nama` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Kelas`
--

LOCK TABLES `Kelas` WRITE;
/*!40000 ALTER TABLE `Kelas` DISABLE KEYS */;
set autocommit=0;
/*!40000 ALTER TABLE `Kelas` ENABLE KEYS */;
UNLOCK TABLES;
commit;

--
-- Table structure for table `Perusahaan`
--

DROP TABLE IF EXISTS `Perusahaan`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `Perusahaan` (
  `Id` int(11) NOT NULL AUTO_INCREMENT,
  `Nama` varchar(255) DEFAULT NULL,
  `Alamat` varchar(255) DEFAULT NULL,
  `Benefit` text DEFAULT NULL,
  `JurusanId` int(11) DEFAULT NULL,
  PRIMARY KEY (`Id`),
  KEY `JurusanId` (`JurusanId`),
  CONSTRAINT `Perusahaan_ibfk_1` FOREIGN KEY (`JurusanId`) REFERENCES `Jurusan` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Perusahaan`
--

LOCK TABLES `Perusahaan` WRITE;
/*!40000 ALTER TABLE `Perusahaan` DISABLE KEYS */;
set autocommit=0;
/*!40000 ALTER TABLE `Perusahaan` ENABLE KEYS */;
UNLOCK TABLES;
commit;

--
-- Table structure for table `Roles`
--

DROP TABLE IF EXISTS `Roles`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `Roles` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `Nama` varchar(50) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Roles`
--

LOCK TABLES `Roles` WRITE;
/*!40000 ALTER TABLE `Roles` DISABLE KEYS */;
set autocommit=0;
INSERT INTO `Roles` VALUES
(1,'Admin');
/*!40000 ALTER TABLE `Roles` ENABLE KEYS */;
UNLOCK TABLES;
commit;

--
-- Table structure for table `Users`
--

DROP TABLE IF EXISTS `Users`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `Users` (
  `Id` int(11) NOT NULL AUTO_INCREMENT,
  `Nama` varchar(255) DEFAULT NULL,
  `Password` varchar(255) DEFAULT NULL,
  `Token` varchar(255) DEFAULT NULL,
  `Roles` int(11) DEFAULT NULL,
  `KelasId` int(11) DEFAULT NULL,
  `JurusanId` int(11) DEFAULT NULL,
  `NamaPembimbing` varchar(255) DEFAULT NULL,
  `TempatPerusahaanId` int(11) DEFAULT NULL,
  `SiswaDibimbing` varchar(255) DEFAULT NULL,
  `JurusanKaprodiId` int(11) DEFAULT NULL,
  `AlamatPerusahaan` varchar(255) DEFAULT NULL,
  `BenefitPerusahaan` text DEFAULT NULL,
  `JurusanPerusahaanId` int(11) DEFAULT NULL,
  PRIMARY KEY (`Id`),
  KEY `Roles` (`Roles`),
  KEY `KelasId` (`KelasId`),
  KEY `JurusanId` (`JurusanId`),
  KEY `TempatPerusahaanId` (`TempatPerusahaanId`),
  KEY `JurusanKaprodiId` (`JurusanKaprodiId`),
  KEY `JurusanPerusahaanId` (`JurusanPerusahaanId`),
  CONSTRAINT `Users_ibfk_1` FOREIGN KEY (`Roles`) REFERENCES `Roles` (`id`),
  CONSTRAINT `Users_ibfk_2` FOREIGN KEY (`KelasId`) REFERENCES `Kelas` (`id`),
  CONSTRAINT `Users_ibfk_3` FOREIGN KEY (`JurusanId`) REFERENCES `Jurusan` (`id`),
  CONSTRAINT `Users_ibfk_4` FOREIGN KEY (`TempatPerusahaanId`) REFERENCES `Perusahaan` (`Id`),
  CONSTRAINT `Users_ibfk_5` FOREIGN KEY (`JurusanKaprodiId`) REFERENCES `Jurusan` (`id`),
  CONSTRAINT `Users_ibfk_6` FOREIGN KEY (`JurusanPerusahaanId`) REFERENCES `Jurusan` (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Users`
--

LOCK TABLES `Users` WRITE;
/*!40000 ALTER TABLE `Users` DISABLE KEYS */;
set autocommit=0;
/*!40000 ALTER TABLE `Users` ENABLE KEYS */;
UNLOCK TABLES;
commit;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*M!100616 SET NOTE_VERBOSITY=@OLD_NOTE_VERBOSITY */;

-- Dump completed on 2025-11-07 11:44:02
