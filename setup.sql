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
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `nama` varchar(255) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Jurusan`
--

LOCK TABLES `Jurusan` WRITE;
/*!40000 ALTER TABLE `Jurusan` DISABLE KEYS */;
set autocommit=0;
INSERT INTO `Jurusan` VALUES
(1,'Super');
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
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `nama` varchar(255) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Kelas`
--

LOCK TABLES `Kelas` WRITE;
/*!40000 ALTER TABLE `Kelas` DISABLE KEYS */;
set autocommit=0;
INSERT INTO `Kelas` VALUES
(1,'Super');
/*!40000 ALTER TABLE `Kelas` ENABLE KEYS */;
UNLOCK TABLES;
commit;

--
-- Table structure for table `Pembimbing`
--

DROP TABLE IF EXISTS `Pembimbing`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `Pembimbing` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `jurusan_id` int(10) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  KEY `jurusan_id_fk` (`jurusan_id`),
  CONSTRAINT `jurusan_id_fk` FOREIGN KEY (`jurusan_id`) REFERENCES `Jurusan` (`id`) ON DELETE NO ACTION ON UPDATE NO ACTION
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Pembimbing`
--

LOCK TABLES `Pembimbing` WRITE;
/*!40000 ALTER TABLE `Pembimbing` DISABLE KEYS */;
set autocommit=0;
/*!40000 ALTER TABLE `Pembimbing` ENABLE KEYS */;
UNLOCK TABLES;
commit;

--
-- Table structure for table `Perusahaan`
--

DROP TABLE IF EXISTS `Perusahaan`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `Perusahaan` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `benefit` text NOT NULL,
  `alamat` text NOT NULL,
  `jurusan_id` int(10) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  KEY `fk_jurusan_id` (`jurusan_id`),
  CONSTRAINT `fk_jurusan_id` FOREIGN KEY (`jurusan_id`) REFERENCES `Jurusan` (`id`) ON DELETE NO ACTION ON UPDATE NO ACTION
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
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `nama` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=6 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Roles`
--

LOCK TABLES `Roles` WRITE;
/*!40000 ALTER TABLE `Roles` DISABLE KEYS */;
set autocommit=0;
INSERT INTO `Roles` VALUES
(1,'Admin'),
(2,'Kaprodi'),
(3,'Perusahaan'),
(4,'Pembimbing'),
(5,'Siswa');
/*!40000 ALTER TABLE `Roles` ENABLE KEYS */;
UNLOCK TABLES;
commit;

--
-- Table structure for table `Siswa`
--

DROP TABLE IF EXISTS `Siswa`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `Siswa` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `kelas_id` int(10) unsigned NOT NULL,
  `jurusan_id` int(10) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  KEY `kelas_id` (`kelas_id`),
  KEY `jurusan_id` (`jurusan_id`),
  CONSTRAINT `jurusan_id` FOREIGN KEY (`jurusan_id`) REFERENCES `Jurusan` (`id`) ON DELETE NO ACTION ON UPDATE NO ACTION,
  CONSTRAINT `kelas_id` FOREIGN KEY (`kelas_id`) REFERENCES `Kelas` (`id`) ON DELETE NO ACTION ON UPDATE NO ACTION
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Siswa`
--

LOCK TABLES `Siswa` WRITE;
/*!40000 ALTER TABLE `Siswa` DISABLE KEYS */;
set autocommit=0;
/*!40000 ALTER TABLE `Siswa` ENABLE KEYS */;
UNLOCK TABLES;
commit;

--
-- Table structure for table `Siswa_Pembimbing`
--

DROP TABLE IF EXISTS `Siswa_Pembimbing`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `Siswa_Pembimbing` (
  `id_siswa` int(10) unsigned NOT NULL,
  `id_pembimbing` int(10) unsigned NOT NULL,
  PRIMARY KEY (`id_siswa`),
  KEY `id_pembimbing_fk` (`id_pembimbing`),
  CONSTRAINT `id_pembimbing_fk` FOREIGN KEY (`id_pembimbing`) REFERENCES `Pembimbing` (`id`) ON DELETE NO ACTION ON UPDATE NO ACTION,
  CONSTRAINT `id_siswa_fk` FOREIGN KEY (`id_siswa`) REFERENCES `Siswa` (`id`) ON DELETE NO ACTION ON UPDATE NO ACTION
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Siswa_Pembimbing`
--

LOCK TABLES `Siswa_Pembimbing` WRITE;
/*!40000 ALTER TABLE `Siswa_Pembimbing` DISABLE KEYS */;
set autocommit=0;
/*!40000 ALTER TABLE `Siswa_Pembimbing` ENABLE KEYS */;
UNLOCK TABLES;
commit;

--
-- Table structure for table `Siswa_Perusahaan`
--

DROP TABLE IF EXISTS `Siswa_Perusahaan`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `Siswa_Perusahaan` (
  `id_siswa` int(10) unsigned NOT NULL,
  `id_perusahaan` int(10) unsigned NOT NULL,
  PRIMARY KEY (`id_siswa`),
  KEY `fk_id_perusahaan` (`id_perusahaan`),
  CONSTRAINT `fk_id_perusahaan` FOREIGN KEY (`id_perusahaan`) REFERENCES `Perusahaan` (`id`) ON DELETE NO ACTION ON UPDATE NO ACTION,
  CONSTRAINT `fk_id_siswa` FOREIGN KEY (`id_siswa`) REFERENCES `Siswa` (`id`) ON DELETE NO ACTION ON UPDATE NO ACTION
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `Siswa_Perusahaan`
--

LOCK TABLES `Siswa_Perusahaan` WRITE;
/*!40000 ALTER TABLE `Siswa_Perusahaan` DISABLE KEYS */;
set autocommit=0;
/*!40000 ALTER TABLE `Siswa_Perusahaan` ENABLE KEYS */;
UNLOCK TABLES;
commit;

--
-- Table structure for table `Users`
--

DROP TABLE IF EXISTS `Users`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8mb4 */;
CREATE TABLE `Users` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `nama` varchar(255) NOT NULL,
  `password` varchar(255) NOT NULL,
  `token` varchar(255) NOT NULL,
  `roles_id` int(10) unsigned NOT NULL,
  PRIMARY KEY (`id`),
  KEY `fk_roles_id` (`roles_id`),
  CONSTRAINT `fk_roles_id` FOREIGN KEY (`roles_id`) REFERENCES `Roles` (`id`) ON DELETE NO ACTION ON UPDATE NO ACTION
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

-- Dump completed on 2025-11-11 14:07:00
